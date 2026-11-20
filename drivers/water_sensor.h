/**
 * @file water_sensor.h
 *
 * @brief Interfaz común para cualquier sensor de nivel de agua.
 *
 * Cada modelo de sensor implementa estas funciones en su propio .c
 * (water_sensor_ky059.c, water_sensor_grove.c, ...). El modelo activo
 * se elige con WATER_SENSOR_MODEL en hardware_config.h.
 *
 * Las capas superiores (app/water_level) solo conocen esta interfaz:
 * cambiar de sensor no obliga a tocar nada fuera de drivers/ y config/.
 */
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa el hardware del sensor (ADC, I2C, GPIO de alimentación...).
 */
esp_err_t water_sensor_init(void);

/**
 * @brief Lee el nivel de agua.
 *
 * @param[out] percent Nivel en % (0.0 - 100.0) del rango útil del sensor.
 *                     Cada driver aplica su propia calibración.
 *
 * @return ESP_OK si la lectura es válida.
 */
esp_err_t water_sensor_read_percent(float *percent);

#ifdef __cplusplus
}
#endif
