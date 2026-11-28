/**
 * @file soil_sensor.h
 *
 * @brief Interfaz común para sensores de humedad del suelo.
 *
 * Implementaciones: soil_sensor_capacitive.c (sensor capacitivo analógico v2.0 / HW-390).
 * El modelo activo se elige con SOIL_SENSOR_MODEL en hardware_config.h.
 */
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t soil_sensor_init(void);

/**
 * @param[out] percent Humedad del suelo en % (0 = seco, 100 = saturado),
 *                     según la calibración del driver.
 */
esp_err_t soil_sensor_read_percent(float *percent);

#ifdef __cplusplus
}
#endif
