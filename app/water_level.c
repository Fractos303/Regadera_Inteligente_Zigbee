#include "water_level.h"

#include "esp_log.h"

static const char *TAG = "WATER_LEVEL";

static float current_level = 75.0f; //HACK Codigo de test para simular el nivel de agua. En la version final, este valor se obtendra del sensor de nivel de agua.

esp_err_t water_level_init(void)
{
    ESP_LOGI(TAG, "Water level initialized");

    return ESP_OK;
}

float water_level_get(void)
{
    return current_level;
}