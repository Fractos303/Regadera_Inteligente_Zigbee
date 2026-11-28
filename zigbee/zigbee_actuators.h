/**
 * @file zigbee_actuators.h
 *
 * @brief Puente entre las órdenes Zigbee y los actuadores (riego).
 *
 * - Recibe la orden On/Off de Home Assistant y la pasa a app/irrigation.
 * - Si el riego se para o se rechaza por una causa interna (depósito vacío,
 *   tiempo máximo, fallo), vuelve a poner el atributo On/Off a OFF para que
 *   el interruptor de HA refleje el estado real.
 * - Ejecuta cada segundo las comprobaciones de seguridad del riego.
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Arranca el ciclo de seguridad (llamar una vez, desde la tarea Zigbee).
 */
void zigbee_actuators_start(void);

/**
 * @brief Procesa la orden On/Off recibida de Home Assistant.
 */
void zigbee_actuators_handle_on_off(bool on);

#ifdef __cplusplus
}
#endif
