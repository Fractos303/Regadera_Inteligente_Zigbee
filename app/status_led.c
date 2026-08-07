/**
 * @file status_led.c
 * @brief Status LED control.
 * @author Fractos
 * @date 2024-06-10
 */

#include "status_led.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ws2812_driver.h"

static const char *TAG = "STATUS_LED";

/*
 * ESP32-H2 DevKitM-1
 * WS2812 integrado conectado al GPIO8.
 */
#define STATUS_LED_GPIO      GPIO_NUM_8
#define STATUS_LED_NUM_LEDS  1

static uint8_t current_red   = 255;
static uint8_t current_green = 255;
static uint8_t current_blue  = 255;

esp_err_t status_led_init(void)
{
    ESP_ERROR_CHECK(ws2812_driver_init(STATUS_LED_GPIO, STATUS_LED_NUM_LEDS));

    ESP_LOGI(TAG, "Status LED initialized");

    return ESP_OK;
}

esp_err_t status_led_set_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
    current_red   = red;
    current_green = green;
    current_blue  = blue;

    return ESP_OK;
}

esp_err_t status_led_on(void)
{
    return ws2812_driver_set_rgb(0, current_red, current_green, current_blue);
}

esp_err_t status_led_off(void)
{
    return ws2812_driver_clear();
}

esp_err_t status_led_blink(uint8_t times, uint32_t period_ms)
{
    ESP_LOGI(TAG, "Blink x%d", times);

    for (uint8_t i = 0; i < times; i++) {

        ESP_ERROR_CHECK(status_led_on());
        vTaskDelay(pdMS_TO_TICKS(period_ms));

        ESP_ERROR_CHECK(status_led_off());
        vTaskDelay(pdMS_TO_TICKS(period_ms));
    }

    return ESP_OK;
}