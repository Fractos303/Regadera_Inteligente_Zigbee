/**
 * @file zigbee_config.h
 * 
 * @brief Zigbee configuration for the smart valve project.
 * 
 * @author Fractos
 * 
 * @date 2024-06-10
 * 
 */
#pragma once

#include "ha/esp_zigbee_ha_standard.h"

/*=========================================================
 * Zigbee Endpoint Configuration
 *========================================================*/

/**
 * Endpoint utilizado por la regadera inteligente.
 */
#define HA_ESP_VALVE_ENDPOINT          10

/**
 * Zigbee Home Automation Profile.
 */
#define HA_PROFILE_ID                  ESP_ZB_AF_HA_PROFILE_ID

/**
 * Tipo de dispositivo Zigbee.
 *
 * De momento utilizamos una bombilla On/Off porque estamos
 * sustituyendo el ejemplo oficial.
 *
 * En el futuro se investigará qué Device ID representa mejor
 * una válvula o un sistema de riego.
 */
#define HA_DEVICE_ID                   ESP_ZB_HA_ON_OFF_SWITCH_DEVICE_ID

/**
 * Versión del dispositivo.
 */
#define HA_DEVICE_VERSION              0


/*=========================================================
 * Basic Cluster Information
 *========================================================*/

/**
 * Fabricante mostrado en Home Assistant.
 */
#define ESP_MANUFACTURER_NAME          "\x08""Fractos"

/**
 * Modelo mostrado en Home Assistant.
 */
#define ESP_MODEL_IDENTIFIER           "\x08""Regadera"