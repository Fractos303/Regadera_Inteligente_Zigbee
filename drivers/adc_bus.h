/**
 * @file adc_bus.h
 *
 * @brief ADC compartido entre varios drivers.
 *
 * El ESP32-H2 solo tiene una unidad ADC (ADC1) y ESP-IDF solo permite
 * inicializarla una vez. Este módulo la crea la primera vez que alguien
 * la pide y deja que cada driver (nivel de agua, LDR...) añada su canal.
 */
#pragma once

#include "esp_err.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    adc_unit_t        unit;
    adc_channel_t     channel;
    adc_cali_handle_t cali;     // NULL si no hay calibración disponible
} adc_bus_channel_t;

/**
 * @brief Configura el GPIO como entrada ADC (atenuación 12 dB, ~0–3,3 V).
 */
esp_err_t adc_bus_add_channel(gpio_num_t gpio, adc_bus_channel_t *out);

/**
 * @brief Lee la tensión media de 'samples' muestras, en mV.
 */
esp_err_t adc_bus_read_mv(const adc_bus_channel_t *ch, int samples, int *mv);

#ifdef __cplusplus
}
#endif
