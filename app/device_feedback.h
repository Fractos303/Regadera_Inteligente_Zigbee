#pragma once

#include "esp_err.h"

/**
 * Señales luminosas del dispositivo (LED WS2812 integrado).
 *
 * Las funciones NO bloquean: encolan el patrón y una tarea propia lo ejecuta.
 * Así se pueden llamar desde la tarea Zigbee sin pararla.
 *
 * | Evento             | Color    | Patrón                 |
 * |--------------------|----------|------------------------|
 * | Arranque           | Azul     | 1 parpadeo lento       |
 * | Unido a la red     | Verde    | 2 parpadeos            |
 * | Salió de la red    | Naranja  | 2 parpadeos            |
 * | Error              | Rojo     | 5 parpadeos rápidos    |
 * | Depósito vacío     | Morado   | 3 parpadeos lentos     |
 * | Identify (HA)      | Blanco   | continuo (identify.c)  |
 */

/**
 * Crea la cola y la tarea del LED. Llamar después de status_led_init().
 */
esp_err_t device_feedback_init(void);

/**
 * El dispositivo se ha unido correctamente a la red Zigbee.
 */
esp_err_t device_feedback_joined(void);

/**
 * El dispositivo ha abandonado la red Zigbee.
 */
esp_err_t device_feedback_left(void);

/**
 * El dispositivo está arrancando.
 */
esp_err_t device_feedback_boot(void);

/**
 * Error del dispositivo.
 */
esp_err_t device_feedback_error(void);

/**
 * Depósito vacío: el riego se ha parado o se ha rechazado.
 */
esp_err_t device_feedback_tank_empty(void);
