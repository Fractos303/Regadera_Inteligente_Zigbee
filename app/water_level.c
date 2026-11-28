#include <stdbool.h>

#include "water_level.h"
#include "water_sensor.h"

#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "WATER_LEVEL";

#define WATER_LEVEL_DEMO          0         // 1 = simulación, 0 = sensor real
#define WATER_LEVEL_FILTER_ALPHA  1.0f      // filtro paso bajo: 1.0 = sin filtro, más bajo = más suave

#define WATER_LEVEL_EMPTY_PCT       5.0f    // por debajo → vacío
#define WATER_LEVEL_REFILLED_PCT   15.0f    // por encima → rellenado

static bool tank_empty = false;

static float current_level = 0.0f;
static bool  has_reading   = false;

esp_err_t water_level_init(void)
{
#if !WATER_LEVEL_DEMO
    ESP_RETURN_ON_ERROR(water_sensor_init(), TAG, "Fallo al iniciar el sensor");
#endif

    ESP_LOGI(TAG, "Water level initialized (%s)", WATER_LEVEL_DEMO ? "DEMO" : "sensor real");
    return ESP_OK;
}

float water_level_update(void)
{
    float new_level;

#if WATER_LEVEL_DEMO
    static float demo_level = 80.0f;
    demo_level -= 5.0f;
    if (demo_level < 0.0f) {
        demo_level = 100.0f;
    }
    new_level = demo_level;
#else
    if (water_sensor_read_percent(&new_level) != ESP_OK) {
        ESP_LOGW(TAG, "Lectura fallida, se mantiene %.1f %%", current_level);
        return current_level;
    }
#endif

    /* Filtro exponencial: suaviza el oleaje y el ruido del ADC */
    if (!has_reading) {
        current_level = new_level;                  // primera lectura sin filtrar
        has_reading   = true;
    } else {
        current_level += WATER_LEVEL_FILTER_ALPHA * (new_level - current_level);
    }

    /* Si aplicaste la detección de eventos de la nota
       "Zigbee - Reporting y eventos inmediatos", va aquí: */
    // detect_events(current_level);

    if (!tank_empty && current_level <= WATER_LEVEL_EMPTY_PCT) {
        tank_empty = true;
    } else if (tank_empty && current_level >= WATER_LEVEL_REFILLED_PCT) {
        tank_empty = false;
    }

    return current_level;
}

float water_level_get(void)
{
    return current_level;
}

bool water_level_is_empty(void)
{
    return has_reading && tank_empty;
}