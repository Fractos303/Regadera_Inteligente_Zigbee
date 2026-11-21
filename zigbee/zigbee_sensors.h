/**
 * @file zigbee_sensors.h
 *
 * @brief Publicación periódica de los sensores en sus atributos Zigbee.
 *
 * Sustituye a water_level_report_cb() de zigbee_stack.c.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Arranca la actualización periódica (llamar una vez, desde la tarea Zigbee).
 */
void zigbee_sensors_start(void);

#ifdef __cplusplus
}
#endif
