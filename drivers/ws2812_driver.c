#include "esp_log.h"
#include "led_strip.h"  //-> library para LEDs WS2812...
#include "ws2812_driver.h"

static led_strip_handle_t s_led_strip;

static const char *TAG = "WS2812";

esp_err_t ws2812_driver_init(gpio_num_t gpio, uint8_t num_leds)
{
    led_strip_config_t led_strip_conf = {
        .max_leds = num_leds,
        .strip_gpio_num = gpio,
    };
    led_strip_rmt_config_t rmt_conf = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&led_strip_conf, &rmt_conf, &s_led_strip)); 

    ESP_LOGI(TAG, "Driver initialized");
    return ESP_OK;
}

esp_err_t ws2812_driver_set_rgb(uint8_t led, uint8_t r, uint8_t g, uint8_t b)
{
    // NOTE: El LED RGB integrado de esta placa utiliza orden GRB 
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, led, g, r, b));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
    ESP_LOGI(TAG, "LED %d -> (%d,%d,%d)", led, r, g, b);
    return ESP_OK;
}

esp_err_t ws2812_driver_clear(void)
{
    ESP_ERROR_CHECK(led_strip_clear(s_led_strip));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
    return ESP_OK;
}

