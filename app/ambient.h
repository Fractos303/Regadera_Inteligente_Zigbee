/**
 * @file ambient.h
 *
 * @brief Condiciones ambientales: temperatura, humedad y luz.
 *
 * Lee los sensores a través de sus interfaces (climate_sensor, light_sensor)
 * y guarda el último valor válido. No sabe qué modelo de sensor hay detrás
 * ni nada de Zigbee.
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ambient_init(void);

/** Lee todos los sensores ambientales. Si uno falla, conserva su último valor. */
void ambient_update(void);

/** Devuelven false si todavía no hay ninguna lectura válida. */
bool ambient_get_temperature(float *celsius);
bool ambient_get_humidity(float *percent);
bool ambient_get_lux(float *lux);

#ifdef __cplusplus
}
#endif
