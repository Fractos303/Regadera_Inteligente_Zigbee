#pragma once

#include "esp_err.h"

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