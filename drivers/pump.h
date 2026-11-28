/**
 * @file pump.h
 *
 * @brief Interfaz común para la bomba de riego.
 *
 * Implementaciones: pump_drv8833.c (bomba peristáltica DC con DRV8833).
 * El modelo activo se elige con PUMP_MODEL en hardware_config.h.
 *
 * La capa de aplicación (app/irrigation) solo conoce esta interfaz:
 * cambiar de bomba o de driver no obliga a tocar nada fuera de drivers/ y config/.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configura los pines y el PWM. La bomba queda parada y el driver dormido.
 */
esp_err_t pump_init(void);

/**
 * @brief Arranca la bomba con arranque suave hasta la velocidad indicada.
 *
 * Si ya está en marcha, cambia la velocidad (subiendo en rampa o bajando directamente).
 *
 * @param speed_pct Velocidad en % (1–100). 0 equivale a pump_stop().
 */
esp_err_t pump_start(uint8_t speed_pct);

/**
 * @brief Para la bomba de inmediato y pone el driver en reposo (bajo consumo).
 */
esp_err_t pump_stop(void);

/**
 * @brief true si la bomba está en marcha.
 */
bool pump_is_running(void);

/**
 * @brief true si el driver indica un fallo (sobrecorriente, temperatura...).
 *        Siempre false si no hay pin de fallo conectado.
 */
bool pump_has_fault(void);

#ifdef __cplusplus
}
#endif
