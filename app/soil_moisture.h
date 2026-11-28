/**
 * @file soil_moisture.h
 *
 * @brief Humedad del suelo de la maceta.
 *
 * Lee el sensor a través de drivers/soil_sensor.h, suaviza la lectura y
 * guarda el último valor válido. No sabe nada de Zigbee.
 *
 * Será la base para el riego automático: "si la tierra baja del X %, regar".
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t soil_moisture_init(void);

/** Lee el sensor. Si falla, conserva el último valor. */
void soil_moisture_update(void);

/** false si todavía no hay ninguna lectura válida. */
bool soil_moisture_get(float *percent);

#ifdef __cplusplus
}
#endif
