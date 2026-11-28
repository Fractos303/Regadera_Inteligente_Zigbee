/**
 * @file pump_drv8833.c
 *
 * @brief Driver de bomba peristáltica DC con el puente en H DRV8833 (canal A).
 *
 * Control (sentido directo, decaimiento rápido):
 *
 *     AIN1 = PWM     AIN2 = 0     → la bomba gira, velocidad = ciclo de trabajo
 *     AIN1 = 0       AIN2 = 0     → parada (salidas libres)
 *
 * nSLEEP: a nivel bajo el DRV8833 entra en reposo (salidas en alta impedancia,
 * consumo de µA). El driver lo despierta al arrancar y lo duerme al parar,
 * pensando ya en la futura alimentación por pilas.
 *
 * nFAULT (opcional): salida en drenador abierto del DRV8833; se pone a 0 ante
 * sobrecorriente o sobretemperatura. Se lee con el pull-up interno.
 *
 * Arranque suave: el ciclo de trabajo sube en pasos de PUMP_RAMP_STEP_MS
 * durante PUMP_SOFT_START_MS, con un esp_timer (no bloquea la tarea Zigbee).
 */
#include "pump.h"
#include "hardware_config.h"

#if PUMP_MODEL == PUMP_DRV8833

#include "esp_log.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "PUMP_DRV8833";

#define PUMP_LEDC_MODE       LEDC_LOW_SPEED_MODE
#define PUMP_LEDC_TIMER      LEDC_TIMER_1          // el timer 0 lo usaba el servo
#define PUMP_LEDC_CHANNEL    LEDC_CHANNEL_1
#define PUMP_LEDC_RES        LEDC_TIMER_10_BIT
#define PUMP_DUTY_MAX        ((1u << 10) - 1)      // 1023
#define PUMP_RAMP_STEP_MS    20

static esp_timer_handle_t ramp_timer  = NULL;
static SemaphoreHandle_t  lock        = NULL;

static uint32_t duty_now    = 0;
static uint32_t duty_target = 0;
static uint32_t duty_step   = 1;
static bool     running     = false;

/* Variable (no macro) para poder comparar con GPIO_NUM_NC sin avisos */
static const gpio_num_t fault_gpio = PUMP_NFAULT_GPIO;

/*---------------------------------------------------------------
 * Utilidades
 *-------------------------------------------------------------*/
static void set_duty(uint32_t duty)
{
    ledc_set_duty(PUMP_LEDC_MODE, PUMP_LEDC_CHANNEL, duty);
    ledc_update_duty(PUMP_LEDC_MODE, PUMP_LEDC_CHANNEL);
    duty_now = duty;
}

static esp_err_t config_output(gpio_num_t gpio, int level)
{
    ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio), TAG, "reset GPIO %d", gpio);
    ESP_RETURN_ON_ERROR(gpio_set_direction(gpio, GPIO_MODE_OUTPUT), TAG, "GPIO %d", gpio);
    return gpio_set_level(gpio, level);
}

/* Rampa de arranque: se ejecuta en la tarea de esp_timer */
static void ramp_cb(void *arg)
{
    xSemaphoreTake(lock, portMAX_DELAY);

    if (running && duty_now < duty_target) {
        uint32_t next = duty_now + duty_step;
        set_duty(next > duty_target ? duty_target : next);
    }

    if (!running || duty_now >= duty_target) {
        esp_timer_stop(ramp_timer);
    }

    xSemaphoreGive(lock);
}

/*---------------------------------------------------------------
 * Inicialización
 *-------------------------------------------------------------*/
esp_err_t pump_init(void)
{
    lock = xSemaphoreCreateMutex();
    ESP_RETURN_ON_FALSE(lock, ESP_ERR_NO_MEM, TAG, "mutex");

    /* AIN2 fijo a 0 → sentido directo */
    ESP_RETURN_ON_ERROR(config_output(PUMP_AIN2_GPIO, 0), TAG, "AIN2");

    /* nSLEEP a 0 → el DRV8833 arranca dormido */
    ESP_RETURN_ON_ERROR(config_output(PUMP_NSLEEP_GPIO, 0), TAG, "nSLEEP");

    /* nFAULT (opcional) */
    if (fault_gpio != GPIO_NUM_NC) {
        ESP_RETURN_ON_ERROR(gpio_reset_pin(fault_gpio), TAG, "nFAULT");
        ESP_RETURN_ON_ERROR(gpio_set_direction(fault_gpio, GPIO_MODE_INPUT), TAG, "nFAULT");
        ESP_RETURN_ON_ERROR(gpio_set_pull_mode(fault_gpio, GPIO_PULLUP_ONLY), TAG, "nFAULT pull-up");
    }

    /* PWM en AIN1 */
    ledc_timer_config_t timer_cfg = {
        .speed_mode      = PUMP_LEDC_MODE,
        .duty_resolution = PUMP_LEDC_RES,
        .timer_num       = PUMP_LEDC_TIMER,
        .freq_hz         = PUMP_PWM_FREQ_HZ,       // 20 kHz: inaudible
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer_cfg), TAG, "LEDC timer");

    ledc_channel_config_t chan_cfg = {
        .gpio_num   = PUMP_AIN1_GPIO,
        .speed_mode = PUMP_LEDC_MODE,
        .channel    = PUMP_LEDC_CHANNEL,
        .timer_sel  = PUMP_LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ESP_RETURN_ON_ERROR(ledc_channel_config(&chan_cfg), TAG, "LEDC canal");

    /* Temporizador de la rampa */
    esp_timer_create_args_t timer_args = {
        .callback = ramp_cb,
        .name     = "pump_ramp",
    };
    ESP_RETURN_ON_ERROR(esp_timer_create(&timer_args, &ramp_timer), TAG, "esp_timer");

    ESP_LOGI(TAG, "Inicializado (AIN1=%d PWM %d Hz, AIN2=%d, nSLEEP=%d, nFAULT=%d)",
             PUMP_AIN1_GPIO, PUMP_PWM_FREQ_HZ, PUMP_AIN2_GPIO, PUMP_NSLEEP_GPIO, fault_gpio);

    return ESP_OK;
}

/*---------------------------------------------------------------
 * Marcha / paro
 *-------------------------------------------------------------*/
esp_err_t pump_start(uint8_t speed_pct)
{
    ESP_RETURN_ON_FALSE(lock, ESP_ERR_INVALID_STATE, TAG, "No inicializado");

    if (speed_pct == 0) {
        return pump_stop();
    }
    if (speed_pct > 100) {
        speed_pct = 100;
    }

    xSemaphoreTake(lock, portMAX_DELAY);

    if (!running) {
        gpio_set_level(PUMP_NSLEEP_GPIO, 1);       // despertar el DRV8833
        esp_rom_delay_us(PUMP_WAKE_US);            // tiempo de despertar (~1 ms)
        set_duty(0);
    }

    duty_target = (uint32_t)speed_pct * PUMP_DUTY_MAX / 100;

    uint32_t steps = PUMP_SOFT_START_MS / PUMP_RAMP_STEP_MS;
    if (steps == 0) {
        steps = 1;
    }
    duty_step = duty_target / steps;
    if (duty_step == 0) {
        duty_step = 1;
    }

    if (duty_now > duty_target) {
        set_duty(duty_target);                     // bajar velocidad: sin rampa
    }

    running = true;

    xSemaphoreGive(lock);

    /* (Re)lanzar la rampa; si ya está en el objetivo, se para sola */
    esp_timer_stop(ramp_timer);                    // error ignorado si no estaba activa
    esp_timer_start_periodic(ramp_timer, PUMP_RAMP_STEP_MS * 1000);

    ESP_LOGI(TAG, "Marcha al %u %%", speed_pct);
    return ESP_OK;
}

esp_err_t pump_stop(void)
{
    ESP_RETURN_ON_FALSE(lock, ESP_ERR_INVALID_STATE, TAG, "No inicializado");

    esp_timer_stop(ramp_timer);                    // error ignorado si no estaba activa

    xSemaphoreTake(lock, portMAX_DELAY);

    bool was_running = running;
    running     = false;
    duty_target = 0;
    set_duty(0);
    gpio_set_level(PUMP_NSLEEP_GPIO, 0);           // dormir el DRV8833

    xSemaphoreGive(lock);

    if (was_running) {
        ESP_LOGI(TAG, "Parada");
    }
    return ESP_OK;
}

bool pump_is_running(void)
{
    return running;
}

bool pump_has_fault(void)
{
    if (fault_gpio == GPIO_NUM_NC) {
        return false;
    }
    return gpio_get_level(fault_gpio) == 0;
}

#endif /* PUMP_MODEL == PUMP_DRV8833 */
