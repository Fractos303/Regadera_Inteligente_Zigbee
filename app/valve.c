#include "valve.h"
#include "servo.h"

#include "esp_log.h"

static const char *TAG = "VALVE";

static valve_state_t current_state = VALVE_CLOSED;

esp_err_t valve_init(void)
{
    ESP_ERROR_CHECK(servo_init());

    current_state = VALVE_CLOSED;

    ESP_LOGI(TAG, "Valve initialized");

    return ESP_OK;
}

esp_err_t valve_set_state(valve_state_t state)
{
    if (state == current_state)
    {
        return ESP_OK;
    }

    if (state == VALVE_OPEN)
    {
        ESP_ERROR_CHECK(servo_open());
        ESP_LOGI(TAG, "Valve OPEN");
    }
    else
    {
        ESP_ERROR_CHECK(servo_close());
        ESP_LOGI(TAG, "Valve CLOSED");
    }

    current_state = state;

    return ESP_OK;
}

valve_state_t valve_get_state(void)
{
    return current_state;
}