/**
 * @file water_sensor_hx711.c
 *
 * @brief Nivel del depósito por PESO: célula de carga + HX711.
 *
 * Implementa water_sensor.h, así que app/water_level y la capa Zigbee
 * no cambian respecto al KY-059.
 *
 * - Una tarea propia lee el HX711 continuamente (~10 lecturas/s) y guarda
 *   la MEDIANA de las últimas HX711_MEDIAN_SAMPLES. La mediana descarta
 *   picos (golpes, vibración de la bomba) mejor que la media.
 * - water_sensor_read_percent() solo devuelve el último valor calculado:
 *   no bloquea la tarea Zigbee.
 *
 * Conversión:
 *     gramos = (raw - HX711_TARE_RAW) / HX711_COUNTS_PER_GRAM
 *     %      = gramos / WATER_TANK_CAPACITY_G · 100
 *
 * La tara se hace con TODO lo que pesa la báscula montado (botella vacía,
 * bomba, tubo, soporte), así que "0 g" = depósito vacío.
 */
#include "water_sensor.h"
#include "hardware_config.h"

#if WATER_SENSOR_MODEL == WATER_SENSOR_HX711

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hx711.h"

static const char *TAG = "WATER_SENSOR_HX711";

#define HX711_MEDIAN_SAMPLES   9        // ~1 s de lecturas a 10 SPS
#define HX711_READ_TIMEOUT_MS  500

static hx711_t dev;

static int32_t window[HX711_MEDIAN_SAMPLES];
static int     window_count = 0;
static int     window_idx   = 0;

static int32_t median_raw = 0;
static bool    has_value  = false;
static portMUX_TYPE value_mux = portMUX_INITIALIZER_UNLOCKED;

/*---------------------------------------------------------------
 * Mediana
 *-------------------------------------------------------------*/
static int cmp_int32(const void *a, const void *b)
{
    int32_t x = *(const int32_t *)a;
    int32_t y = *(const int32_t *)b;
    return (x > y) - (x < y);
}

static int32_t window_median(void)
{
    int32_t sorted[HX711_MEDIAN_SAMPLES];
    memcpy(sorted, window, window_count * sizeof(int32_t));
    qsort(sorted, window_count, sizeof(int32_t), cmp_int32);
    return sorted[window_count / 2];
}

/*---------------------------------------------------------------
 * Tarea de lectura continua
 *-------------------------------------------------------------*/
static void hx711_task(void *arg)
{
    bool warned = false;

    while (true) {
        int32_t raw;

        if (hx711_read_raw(&dev, &raw, HX711_READ_TIMEOUT_MS) != ESP_OK) {
            if (!warned) {
                ESP_LOGW(TAG, "El HX711 no responde: revisa DT, SCK y alimentación");
                warned = true;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        warned = false;

        window[window_idx] = raw;
        window_idx = (window_idx + 1) % HX711_MEDIAN_SAMPLES;
        if (window_count < HX711_MEDIAN_SAMPLES) {
            window_count++;
        }

        int32_t med = window_median();

        portENTER_CRITICAL(&value_mux);
        median_raw = med;
        has_value  = true;
        portEXIT_CRITICAL(&value_mux);
    }
}

/*---------------------------------------------------------------
 * Interfaz water_sensor.h
 *-------------------------------------------------------------*/
esp_err_t water_sensor_init(void)
{
    dev = (hx711_t){
        .dout = HX711_DOUT_GPIO,
        .sck  = HX711_SCK_GPIO,
        .gain = HX711_GAIN_A_128,
    };

    ESP_RETURN_ON_ERROR(hx711_init(&dev), TAG, "HX711");

    ESP_RETURN_ON_FALSE(xTaskCreate(hx711_task, "hx711", 3072, NULL, 4, NULL) == pdPASS,
                        ESP_ERR_NO_MEM, TAG, "Tarea HX711");

    ESP_LOGI(TAG, "Inicializado (tara=%d, %.2f cuentas/g, depósito=%.0f g)",
             HX711_TARE_RAW, (double)HX711_COUNTS_PER_GRAM, (double)WATER_TANK_CAPACITY_G);
    return ESP_OK;
}

esp_err_t water_sensor_read_percent(float *percent)
{
    ESP_RETURN_ON_FALSE(percent, ESP_ERR_INVALID_ARG, TAG, "percent NULL");

    int32_t raw;
    bool    ok;

    portENTER_CRITICAL(&value_mux);
    raw = median_raw;
    ok  = has_value;
    portEXIT_CRITICAL(&value_mux);

    if (!ok) {
        return ESP_ERR_INVALID_STATE;               // aún no hay ninguna lectura
    }

    float grams = (float)(raw - HX711_TARE_RAW) / HX711_COUNTS_PER_GRAM;
    float pct   = grams * 100.0f / WATER_TANK_CAPACITY_G;

    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;

#if WATER_SENSOR_LOG_RAW
    ESP_LOGI(TAG, "raw=%" PRId32 " -> %.0f g -> %.1f %%", raw, grams, pct);
#endif

    *percent = pct;
    return ESP_OK;
}

#endif /* WATER_SENSOR_MODEL == WATER_SENSOR_HX711 */
