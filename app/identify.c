/**
 * @file identify.c
 * @brief Identify cluster implementation for the smart valve project.
 * @author Fractos
 * @date 2024-06-10
 */

#include "identify.h"

#include "esp_log.h"

#include "status_led.h"

static const char *TAG = "IDENTIFY";

esp_err_t identify_init(void)
{
    ESP_LOGI(TAG, "Identify initialized");
    return ESP_OK;
}

esp_err_t identify_start(void)
{
    ESP_LOGI(TAG, "Identify requested");

    status_led_set_rgb(255,255,255);
    status_led_blink(3, 200);

    return ESP_OK;
}