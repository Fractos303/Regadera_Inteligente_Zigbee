/***
 * @file identify.h
 * @brief Identify cluster interface for the smart valve project.
 * @author Fractos
 * @date 2024-06-10
 */

#pragma once

#include "esp_err.h"

/**
 * Inicializa el módulo Identify.
 */
esp_err_t identify_init(void);

/**
 * Ejecuta el patrón de identificación del dispositivo.
 */
esp_err_t identify_start(void);