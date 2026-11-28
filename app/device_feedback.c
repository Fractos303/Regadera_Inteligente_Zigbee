#include "device_feedback.h"

#include <stdint.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "status_led.h"

static const char *TAG = "DEVICE_FEEDBACK";

typedef struct {
    const char *name;
    uint8_t     red;
    uint8_t     green;
    uint8_t     blue;
    uint8_t     times;
    uint16_t    period_ms;
} feedback_pattern_t;

static QueueHandle_t queue = NULL;

/* Tarea del LED: ejecuta los patrones de uno en uno, en orden de llegada */
static void feedback_task(void *arg)
{
    feedback_pattern_t p;

    while (true) {
        if (xQueueReceive(queue, &p, portMAX_DELAY) == pdTRUE) {
            status_led_set_rgb(p.red, p.green, p.blue);
            status_led_blink(p.times, p.period_ms);   // bloquea esta tarea, no la de Zigbee
        }
    }
}

static esp_err_t play(feedback_pattern_t p)
{
    ESP_LOGI(TAG, "%s", p.name);

    if (queue == NULL) {
        return ESP_ERR_INVALID_STATE;                  // aún no inicializado
    }
    return (xQueueSend(queue, &p, 0) == pdTRUE) ? ESP_OK : ESP_ERR_TIMEOUT;
}

esp_err_t device_feedback_init(void)
{
    if (queue != NULL) {
        return ESP_OK;
    }

    queue = xQueueCreate(8, sizeof(feedback_pattern_t));
    if (queue == NULL) {
        return ESP_ERR_NO_MEM;
    }

    if (xTaskCreate(feedback_task, "feedback", 2560, NULL, 3, NULL) != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Device feedback initialized");
    return ESP_OK;
}

esp_err_t device_feedback_joined(void)
{
    return play((feedback_pattern_t){ "Joined network", 0, 255, 0, 2, 150 });
}

esp_err_t device_feedback_left(void)
{
    return play((feedback_pattern_t){ "Left network", 255, 120, 0, 2, 200 });
}

esp_err_t device_feedback_boot(void)
{
    return play((feedback_pattern_t){ "Boot", 0, 0, 255, 1, 300 });
}

esp_err_t device_feedback_error(void)
{
    return play((feedback_pattern_t){ "Error", 255, 0, 0, 5, 120 });
}

esp_err_t device_feedback_tank_empty(void)
{
    return play((feedback_pattern_t){ "Tank empty", 160, 0, 255, 3, 400 });
}
