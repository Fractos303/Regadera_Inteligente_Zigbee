/***
 * LED driver interface
 * @file led.h
 * @brief This header file defines the interface for controlling an LED in the smart valve project.
 * 
 * @author Fractos
 * @date 2024-06-10
 */

#pragma once

#include "esp_err.h"

esp_err_t led_init(void);

esp_err_t led_on(void);

esp_err_t led_off(void);

esp_err_t led_toggle(void);

esp_err_t led_blink(uint8_t times, uint32_t period_ms);