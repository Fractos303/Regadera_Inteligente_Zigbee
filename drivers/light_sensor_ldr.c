/**
 * @file light_sensor_ldr.c
 *
 * @brief Driver de LDR (fotorresistencia) en divisor de tensión, p. ej. KY-018.
 *
 *   LDR_ON_HIGH_SIDE = 0:   3V3 ── R_fija ──┬── LDR ── GND     (tapar → sube la tensión)
 *                                           S
 *   LDR_ON_HIGH_SIDE = 1:   3V3 ── LDR ─────┬── R_fija ── GND  (tapar → baja la tensión)
 *                                           S
 *
 * Con la tensión se calcula la resistencia del LDR y con ella los lux,
 * usando la curva típica de un GL55xx:
 *
 *     lux = LUX_REF · (R_REF / R_ldr) ^ (1 / GAMMA)
 *
 * Es una APROXIMACIÓN: cada LDR varía bastante. Sirve para distinguir
 * noche / interior / sol directo, no como luxómetro.
 */
#include "light_sensor.h"
#include "hardware_config.h"

#if LIGHT_SENSOR_MODEL == LIGHT_SENSOR_LDR

#include <math.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_check.h"

#include "adc_bus.h"

static const char *TAG = "LIGHT_LDR";

#define LDR_VCC_MV      3300
#define LDR_SAMPLES     8
#define LUX_MAX         100000.0f

static adc_bus_channel_t adc_ch;
static bool              initialized = false;

esp_err_t light_sensor_init(void)
{
    ESP_RETURN_ON_ERROR(adc_bus_add_channel(LIGHT_SENSOR_ADC_GPIO, &adc_ch), TAG, "Canal ADC");
    initialized = true;

    ESP_LOGI(TAG, "Inicializado (GPIO %d, LDR en lado %s)",
             LIGHT_SENSOR_ADC_GPIO, LDR_ON_HIGH_SIDE ? "alto" : "bajo");
    return ESP_OK;
}

esp_err_t light_sensor_read_lux(float *lux)
{
    ESP_RETURN_ON_FALSE(lux,         ESP_ERR_INVALID_ARG,   TAG, "lux NULL");
    ESP_RETURN_ON_FALSE(initialized, ESP_ERR_INVALID_STATE, TAG, "No inicializado");

    int mv;
    ESP_RETURN_ON_ERROR(adc_bus_read_mv(&adc_ch, LDR_SAMPLES, &mv), TAG, "Lectura ADC");

    /* Evita divisiones por cero en los extremos */
    if (mv < 5)               mv = 5;
    if (mv > LDR_VCC_MV - 5)  mv = LDR_VCC_MV - 5;

    /* Resistencia del LDR a partir del divisor */
#if LDR_ON_HIGH_SIDE
    float r_ldr = (float)LDR_R_FIXED * (float)(LDR_VCC_MV - mv) / (float)mv;
#else
    float r_ldr = (float)LDR_R_FIXED * (float)mv / (float)(LDR_VCC_MV - mv);
#endif

    float l = LDR_LUX_REF * powf((float)LDR_R_REF / r_ldr, 1.0f / LDR_GAMMA);

    if (l < 0.0f)    l = 0.0f;
    if (l > LUX_MAX) l = LUX_MAX;

#if LIGHT_SENSOR_LOG_RAW
    ESP_LOGI(TAG, "%d mV -> R=%.0f Ω -> %.0f lx", mv, r_ldr, l);
#endif

    *lux = l;
    return ESP_OK;
}

#endif /* LIGHT_SENSOR_MODEL == LIGHT_SENSOR_LDR */
