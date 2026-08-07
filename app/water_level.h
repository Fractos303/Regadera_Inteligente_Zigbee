#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa el módulo de nivel de agua.
 */
esp_err_t water_level_init(void);

/**
 * @brief Obtiene el nivel de agua.
 *
 * @return Nivel de agua en porcentaje (0.0 - 100.0).
 */
float water_level_get(void);

#ifdef __cplusplus
}
#endif