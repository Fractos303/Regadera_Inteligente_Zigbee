/***
 * @file identify.h
 * @brief Identify cluster interface for the smart valve project.
 * @author Fractos
 * @date 2024-06-10
 */

#pragma once

#include "esp_err.h"
#include <stdint.h>
#include "freertos/FreeRTOS.h"

/**
 * Inicializa el módulo Identify.
 */
esp_err_t identify_init(void);

/**
 * Ejecuta el patrón de identificación del dispositivo.
 */
esp_err_t identify_start(uint16_t identify_time);

esp_err_t identify_stop(void);