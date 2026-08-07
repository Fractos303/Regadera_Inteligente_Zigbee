/***
 * LED driver interface
 * @file led.c
 * @brief //TODO
 * 
 * @author Fractos
 * @date 2024-06-10
 */

#include "led.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LED_GPIO GPIO_NUM_8

static const char *TAG = "LED";

static bool led_state = false;

esp_err_t led_init(void)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_level(LED_GPIO, 0);

    led_state = false;

    ESP_LOGI(TAG, "LED initialized");
    return ESP_OK;
}

esp_err_t led_on(void)
{
    led_state = true;
    gpio_set_level(LED_GPIO, 1);

    ESP_LOGI(TAG, "LED turned on");
    return ESP_OK;
}

esp_err_t led_off(void)
{
    led_state = false;
    gpio_set_level(LED_GPIO, 0);

    ESP_LOGI(TAG, "LED turned off");
    return ESP_OK;
}

esp_err_t led_toggle(void)
{
    led_state = !led_state;
    gpio_set_level(LED_GPIO, led_state);

    return ESP_OK;
}

esp_err_t led_blink(uint8_t times, uint32_t period_ms)
{
    ESP_LOGI(TAG, "Blink");

    led_on();

    vTaskDelay(pdMS_TO_TICKS(5000));

    led_off();

    // ESP_LOGI(TAG, "Blink x%d", times);

    // for(uint8_t i = 0; i < times; i++)
    // {
    //     led_on();
    //     vTaskDelay(pdMS_TO_TICKS(period_ms));

    //     led_off();
    //     vTaskDelay(pdMS_TO_TICKS(period_ms));
    // }

    return ESP_OK;
}