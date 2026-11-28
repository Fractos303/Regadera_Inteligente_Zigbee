#include "soil_moisture.h"

#include "esp_log.h"

#include "soil_sensor.h"

static const char *TAG = "SOIL_MOISTURE";

/* La humedad de la tierra cambia despacio: un filtro suave elimina el ruido
 * del ADC sin retrasar nada que importe. 1.0 = sin filtro. */
#define SOIL_FILTER_ALPHA   0.3f

static float moisture   = 0.0f;
static bool  has_value  = false;
static bool  sensor_ok  = false;

esp_err_t soil_moisture_init(void)
{
    /* Si el sensor falla al iniciar, el resto del dispositivo sigue funcionando */
    sensor_ok = (soil_sensor_init() == ESP_OK);

    if (!sensor_ok) {
        ESP_LOGW(TAG, "Sensor de humedad del suelo no disponible");
    }

    ESP_LOGI(TAG, "Soil moisture initialized");
    return ESP_OK;
}

void soil_moisture_update(void)
{
    if (!sensor_ok) {
        return;
    }

    float value;
    if (soil_sensor_read_percent(&value) != ESP_OK) {
        return;
    }

    if (!has_value) {
        moisture  = value;                  // primera lectura sin filtrar
        has_value = true;
    } else {
        moisture += SOIL_FILTER_ALPHA * (value - moisture);
    }
}

bool soil_moisture_get(float *percent)
{
    if (has_value && percent) {
        *percent = moisture;
    }
    return has_value;
}
