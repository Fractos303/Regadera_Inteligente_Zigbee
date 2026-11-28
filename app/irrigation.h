/**
 * @file irrigation.h
 *
 * @brief Lógica de riego: decide si la bomba puede funcionar y la protege.
 *
 * Sustituye a app/valve. Usa la bomba a través de drivers/pump.h y el nivel
 * del depósito a través de app/water_level.h. No sabe nada de Zigbee: cuando
 * el riego se para (o se rechaza) por una causa interna, avisa mediante un
 * callback para que la capa de comunicación actualice Home Assistant.
 *
 * Seguridades:
 *   - Depósito vacío  → no arranca y, si está regando, se para (con histéresis).
 *   - Tiempo máximo   → se para sola aunque nadie la apague.
 *   - Fallo del driver → se para.
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IRRIGATION_REASON_TANK_EMPTY = 0,   // depósito vacío
    IRRIGATION_REASON_TIMEOUT,          // superado el tiempo máximo de riego
    IRRIGATION_REASON_FAULT,            // fallo del driver de la bomba
} irrigation_reason_t;

/**
 * @brief Se llama SOLO cuando el estado cambia (o una orden se rechaza)
 *        por una causa interna, no por una orden normal.
 *
 * @param running Nuevo estado del riego (normalmente false).
 */
typedef void (*irrigation_state_cb_t)(bool running, irrigation_reason_t reason);

esp_err_t irrigation_init(void);

void irrigation_register_state_cb(irrigation_state_cb_t cb);

/**
 * @brief Orden de encender o apagar el riego.
 *
 * @return ESP_OK, o ESP_ERR_NOT_ALLOWED si una seguridad lo impide.
 */
esp_err_t irrigation_request(bool on);

/**
 * @brief Comprobaciones periódicas de seguridad. Llamar cada ~1 s.
 */
void irrigation_tick(void);

bool irrigation_is_running(void);

#ifdef __cplusplus
}
#endif
