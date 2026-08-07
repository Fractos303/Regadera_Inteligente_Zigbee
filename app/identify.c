/**
 * @file identify.c
 * @brief Identify cluster implementation for the smart valve project.
 * @author Fractos
 * @date 2024-06-10
 */

#include "identify.h"

#include "esp_log.h"
#include "status_led.h"

#define IDENTIFY_RED      255
#define IDENTIFY_GREEN    255
#define IDENTIFY_BLUE     255

static const char *TAG = "IDENTIFY";

static TaskHandle_t identify_task = NULL;

static bool identify_running = false;

static void identify_task_fn(void *arg)
{
    while (identify_running)
    {
        status_led_on();

        vTaskDelay(pdMS_TO_TICKS(250));

        status_led_off();

        vTaskDelay(pdMS_TO_TICKS(250));
    }

    identify_task = NULL;

    status_led_off();

    vTaskDelete(NULL);
}

esp_err_t identify_init(void)
{
    ESP_LOGI(TAG, "Identify initialized");
    return ESP_OK;
}

esp_err_t identify_start(uint16_t identify_time)
{
    ESP_LOGI(TAG, "Identify requested");

    /*
    * Home Assistant actualiza IdentifyTime una vez por segundo.
    * Sólo se crea una tarea cuando comienza la identificación.
    */
    if (identify_task != NULL)
    {
        return ESP_OK;
    }

    status_led_set_rgb(IDENTIFY_RED, IDENTIFY_GREEN, IDENTIFY_BLUE);

    if (!identify_running)
    {
        identify_running = true;
        xTaskCreate(identify_task_fn, "identify", 2048, NULL, 5, &identify_task);
    }

    return ESP_OK;
}

esp_err_t identify_stop(void)
{
    ESP_LOGI(TAG, "Identify stopped");
    identify_running = false;

    if (identify_task == NULL)
    {
        status_led_off();
    }

    return ESP_OK;
}