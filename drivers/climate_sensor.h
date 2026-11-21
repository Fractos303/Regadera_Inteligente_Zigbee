/**
 * @file climate_sensor.h
 *
 * @brief Interfaz común para sensores de temperatura y humedad.
 *
 * Implementaciones: climate_sensor_dht11.c (y en el futuro DHT22, SHT31...).
 * El modelo activo se elige con CLIMATE_SENSOR_MODEL en hardware_config.h.
 */
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t climate_sensor_init(void);

/**
 * @param[out] temperature_c Temperatura en °C.
 * @param[out] humidity_pct  Humedad relativa en %.
 */
esp_err_t climate_sensor_read(float *temperature_c, float *humidity_pct);

#ifdef __cplusplus
}
#endif
