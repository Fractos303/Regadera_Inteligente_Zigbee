/**
 * @file light_sensor.h
 *
 * @brief Interfaz común para sensores de luz.
 *
 * Implementaciones: light_sensor_ldr.c (y en el futuro BH1750, VEML7700...).
 * El modelo activo se elige con LIGHT_SENSOR_MODEL en hardware_config.h.
 */
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t light_sensor_init(void);

/**
 * @param[out] lux Iluminancia en lux (aproximada en el caso del LDR).
 */
esp_err_t light_sensor_read_lux(float *lux);

#ifdef __cplusplus
}
#endif
