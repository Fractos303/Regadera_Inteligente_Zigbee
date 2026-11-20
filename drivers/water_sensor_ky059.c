/**
 * @file water_sensor_ky059.c
 *
 * @brief Driver del sensor de nivel resistivo KY-059 / Funduino "Water Sensor".
 *
 * Salida analógica (pin S): la tensión sube cuanto más pistas están sumergidas.
 * Se lee con el ADC en modo oneshot, se promedian varias muestras y se
 * convierte a % con dos puntos de calibración (vacío / lleno).
 *
 * ⚠ @warning: Alimentar a 3,3 V, NUNCA a 5 V: la salida podría superar el máximo del ADC.
 */
#include "water_sensor.h"
#include "hardware_config.h"

#if WATER_SENSOR_MODEL == WATER_SENSOR_KY059

#include <stdbool.h>

#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "WATER_SENSOR_KY059";

static adc_oneshot_unit_handle_t adc_handle  = NULL;
static adc_cali_handle_t         cali_handle = NULL;
static adc_channel_t             adc_channel;

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
    adc_unit_t unit;

    /* Deduce unidad y canal ADC a partir del GPIO configurado */
    ESP_RETURN_ON_ERROR(
        adc_oneshot_io_to_channel(WATER_SENSOR_ADC_GPIO, &unit, &adc_channel),
        TAG, "GPIO %d no tiene ADC", WATER_SENSOR_ADC_GPIO);

    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = unit,
    };
    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&unit_cfg, &adc_handle),
                        TAG, "No se pudo crear la unidad ADC");

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,                // rango hasta ~3,3 V
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, adc_channel, &chan_cfg),
                        TAG, "No se pudo configurar el canal ADC");

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    /* Calibración de fábrica (eFuse) para convertir raw -> mV con precisión */
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id  = unit,
        .chan     = adc_channel,
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle) != ESP_OK) {
        ESP_LOGW(TAG, "Sin calibración ADC: los mV serán aproximados");
        cali_handle = NULL;
    }
#endif

    if (power_gpio != GPIO_NUM_NC) {
        ESP_RETURN_ON_ERROR(gpio_set_direction(power_gpio, GPIO_MODE_OUTPUT),
                            TAG, "GPIO de alimentación");
        gpio_set_level(power_gpio, 0);              // apagado entre lecturas
    }

    ESP_LOGI(TAG, "Inicializado (GPIO %d, canal ADC %d, vacío=%d mV, lleno=%d mV)",
             WATER_SENSOR_ADC_GPIO, adc_channel,
             WATER_SENSOR_MV_EMPTY, WATER_SENSOR_MV_FULL);

    return ESP_OK;
}

/*---------------------------------------------------------------
 * Lectura
 *-------------------------------------------------------------*/
esp_err_t water_sensor_read_percent(float *percent)
{
    ESP_RETURN_ON_FALSE(percent,    ESP_ERR_INVALID_ARG,   TAG, "percent NULL");
    ESP_RETURN_ON_FALSE(adc_handle, ESP_ERR_INVALID_STATE, TAG, "Sensor no inicializado");

    sensor_power(true);

    int sum_mv = 0;
    int valid  = 0;

    for (int i = 0; i < WATER_SENSOR_SAMPLES; i++) {
        int raw;
        if (adc_oneshot_read(adc_handle, adc_channel, &raw) != ESP_OK) {
            continue;
        }

        int mv;
        if (cali_handle) {
            adc_cali_raw_to_voltage(cali_handle, raw, &mv);
        } else {
            mv = raw * 3300 / 4095;                 // aproximación sin calibrar
        }

        sum_mv += mv;
        valid++;
    }

    sensor_power(false);

    ESP_RETURN_ON_FALSE(valid > 0, ESP_FAIL, TAG, "Ninguna lectura ADC válida");

    int mv = sum_mv / valid;

    /* Conversión lineal mV -> % con los dos puntos de calibración */
    float pct = (float)(mv - WATER_SENSOR_MV_EMPTY) * 100.0f
              / (float)(WATER_SENSOR_MV_FULL - WATER_SENSOR_MV_EMPTY);

    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;

#if WATER_SENSOR_LOG_RAW
    ESP_LOGI(TAG, "%d mV -> %.1f %%", mv, pct);     // útil para calibrar
#endif

    *percent = pct;
    return ESP_OK;
}

#endif /* WATER_SENSOR_MODEL == WATER_SENSOR_KY059 */
