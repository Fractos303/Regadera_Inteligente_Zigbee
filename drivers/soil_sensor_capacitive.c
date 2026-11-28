/**
 * @file soil_sensor_capacitive.c
 *
 * @brief Sensor capacitivo de humedad del suelo v2.0 (HW-390), salida analógica.
 *
 * Funcionamiento: el sensor mide la capacidad entre sus pistas (que NO tocan
 * la tierra, por eso no se corroe). Cuanta más agua hay alrededor, mayor es
 * la capacidad y MENOR la tensión de salida:
 *
 *     seco (aire)  → tensión alta  (~2,5–2,8 V)
 *     mojado       → tensión baja  (~1,1–1,5 V)
 *
 * Conversión lineal con dos puntos de calibración (SOIL_MV_DRY / SOIL_MV_WET).
 *
 * Usa el ADC compartido (adc_bus), igual que el LDR.
 */
#include "soil_sensor.h"
#include "hardware_config.h"

#if SOIL_SENSOR_MODEL == SOIL_SENSOR_CAPACITIVE

#include <stdbool.h>

#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "adc_bus.h"

static const char *TAG = "SOIL_CAPACITIVE";

static adc_bus_channel_t adc_ch;
static bool              initialized = false;

/* GPIO de alimentación opcional (GPIO_NUM_NC = alimentado fijo) */
static const gpio_num_t power_gpio = SOIL_SENSOR_POWER_GPIO;

static void sensor_power(bool on)
{
    if (power_gpio == GPIO_NUM_NC) {
        return;
    }
    gpio_set_level(power_gpio, on ? 1 : 0);
    if (on) {
        vTaskDelay(pdMS_TO_TICKS(SOIL_SENSOR_POWER_SETTLE_MS));   // el oscilador tarda en estabilizarse
    }
}

esp_err_t soil_sensor_init(void)
{
    ESP_RETURN_ON_ERROR(adc_bus_add_channel(SOIL_SENSOR_ADC_GPIO, &adc_ch), TAG, "Canal ADC");

    if (power_gpio != GPIO_NUM_NC) {
        ESP_RETURN_ON_ERROR(gpio_reset_pin(power_gpio), TAG, "GPIO alimentación");
        ESP_RETURN_ON_ERROR(gpio_set_direction(power_gpio, GPIO_MODE_OUTPUT), TAG, "GPIO alimentación");
        gpio_set_level(power_gpio, 0);
    }

    initialized = true;

    ESP_LOGI(TAG, "Inicializado (GPIO %d, seco=%d mV, mojado=%d mV)",
             SOIL_SENSOR_ADC_GPIO, SOIL_MV_DRY, SOIL_MV_WET);
    return ESP_OK;
}

esp_err_t soil_sensor_read_percent(float *percent)
{
    ESP_RETURN_ON_FALSE(percent,     ESP_ERR_INVALID_ARG,   TAG, "percent NULL");
    ESP_RETURN_ON_FALSE(initialized, ESP_ERR_INVALID_STATE, TAG, "No inicializado");

    int mv;

    sensor_power(true);
    esp_err_t err = adc_bus_read_mv(&adc_ch, SOIL_SENSOR_SAMPLES, &mv);
    sensor_power(false);

    ESP_RETURN_ON_ERROR(err, TAG, "Lectura ADC");

    /* Escala invertida: más tensión = más seco */
    float pct = (float)(SOIL_MV_DRY - mv) * 100.0f / (float)(SOIL_MV_DRY - SOIL_MV_WET);

    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;

#if SOIL_SENSOR_LOG_RAW
    ESP_LOGI(TAG, "%d mV -> %.1f %%", mv, pct);
#endif

    *percent = pct;
    return ESP_OK;
}

#endif /* SOIL_SENSOR_MODEL == SOIL_SENSOR_CAPACITIVE */
