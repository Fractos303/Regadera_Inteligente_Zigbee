/**
 * @file water_sensor_ky059.c
 *
 * @brief Driver del sensor de nivel resistivo KY-059 / Funduino "Water Sensor".
 *
 * Salida analógica (pin S): la tensión sube cuanto más pistas están sumergidas.
 * Se lee con el ADC compartido (adc_bus), se promedian varias muestras y se
 * convierte a % con dos puntos de calibración (vacío / lleno).
 *
 * ⚠ Alimentar a 3,3 V, NUNCA a 5 V: la salida podría superar el máximo del ADC.
 */
#include "water_sensor.h"
#include "hardware_config.h"

#if WATER_SENSOR_MODEL == WATER_SENSOR_KY059

#include <stdbool.h>

#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "adc_bus.h"

static const char *TAG = "WATER_SENSOR_KY059";

static adc_bus_channel_t adc_ch;
static bool              initialized = false;

/* Variable (no macro) para evitar avisos de compilación con GPIO_NUM_NC (-1) */
static const gpio_num_t power_gpio = WATER_SENSOR_POWER_GPIO;

/*---------------------------------------------------------------
 * Alimentación conmutada (opcional, reduce la corrosión)
 *-------------------------------------------------------------*/
static void sensor_power(bool on)
{
    if (power_gpio == GPIO_NUM_NC) {
        return;                                     // alimentado fijo a 3V3
    }

    gpio_set_level(power_gpio, on ? 1 : 0);

    if (on) {
        vTaskDelay(pdMS_TO_TICKS(WATER_SENSOR_POWER_SETTLE_MS));
    }
}

/*---------------------------------------------------------------
 * Inicialización
 *-------------------------------------------------------------*/
esp_err_t water_sensor_init(void)
{
    ESP_RETURN_ON_ERROR(adc_bus_add_channel(WATER_SENSOR_ADC_GPIO, &adc_ch),
                        TAG, "Canal ADC");

    if (power_gpio != GPIO_NUM_NC) {
        ESP_RETURN_ON_ERROR(gpio_reset_pin(power_gpio), TAG, "GPIO de alimentación");
        ESP_RETURN_ON_ERROR(gpio_set_direction(power_gpio, GPIO_MODE_OUTPUT), TAG, "GPIO de alimentación");
        gpio_set_level(power_gpio, 0);              // apagado entre lecturas
    }

    initialized = true;

    ESP_LOGI(TAG, "Inicializado (GPIO %d, vacío=%d mV, lleno=%d mV)",
             WATER_SENSOR_ADC_GPIO, WATER_SENSOR_MV_EMPTY, WATER_SENSOR_MV_FULL);

    return ESP_OK;
}

/*---------------------------------------------------------------
 * Lectura
 *-------------------------------------------------------------*/
esp_err_t water_sensor_read_percent(float *percent)
{
    ESP_RETURN_ON_FALSE(percent,     ESP_ERR_INVALID_ARG,   TAG, "percent NULL");
    ESP_RETURN_ON_FALSE(initialized, ESP_ERR_INVALID_STATE, TAG, "Sensor no inicializado");

    int mv;

    sensor_power(true);
    esp_err_t err = adc_bus_read_mv(&adc_ch, WATER_SENSOR_SAMPLES, &mv);
    sensor_power(false);

    ESP_RETURN_ON_ERROR(err, TAG, "Lectura ADC");

    /* Conversión lineal mV -> % con los dos puntos de calibración */
    float pct = (float)(mv - WATER_SENSOR_MV_EMPTY) * 100.0f
              / (float)(WATER_SENSOR_MV_FULL - WATER_SENSOR_MV_EMPTY);

    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;

#if WATER_SENSOR_LOG_RAW
    ESP_LOGI(TAG, "%d mV -> %.1f %%", mv, pct);
#endif

    *percent = pct;
    return ESP_OK;
}

#endif /* WATER_SENSOR_MODEL == WATER_SENSOR_KY059 */
