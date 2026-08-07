#include "device_feedback.h"

#include "esp_log.h"

#include "status_led.h"

static const char *TAG = "DEVICE_FEEDBACK";

esp_err_t device_feedback_joined(void)
{
    ESP_LOGI(TAG, "Joined network");

    status_led_set_rgb(0, 255, 0);
    status_led_blink(2, 150);

    return ESP_OK;
}

esp_err_t device_feedback_left(void)
{
    ESP_LOGI(TAG, "Left network");

    status_led_set_rgb(255, 120, 0);
    status_led_blink(2, 200);

    return ESP_OK;
}

esp_err_t device_feedback_boot(void)
{
    ESP_LOGI(TAG, "Boot");

    status_led_set_rgb(0, 0, 255);
    status_led_blink(1, 300);

    return ESP_OK;
}

esp_err_t device_feedback_error(void)
{
    ESP_LOGI(TAG, "Error");

    status_led_set_rgb(255, 0, 0);
    status_led_blink(5, 120);

    return ESP_OK;
}