#include "ambient.h"

#include "esp_log.h"

#include "climate_sensor.h"
#include "light_sensor.h"

#include "hardware_config.h"

static const char *TAG = "AMBIENT";

static float temperature = 0.0f;
static float humidity    = 0.0f;
static float lux         = 0.0f;

static bool has_climate = false;
static bool has_light   = false;

static bool climate_ok = false;     // sensor inicializado correctamente
static bool light_ok   = false;

esp_err_t ambient_init(void)
{
    /* Un sensor que falla al iniciar no bloquea el resto del dispositivo */
    climate_ok = (climate_sensor_init() == ESP_OK);
    light_ok   = (light_sensor_init()   == ESP_OK);

    if (!climate_ok) ESP_LOGW(TAG, "Sensor de temperatura/humedad no disponible");
    if (!light_ok)   ESP_LOGW(TAG, "Sensor de luz no disponible");

    ESP_LOGI(TAG, "Ambient initialized");
    return ESP_OK;
}

void ambient_update(void)
{
    if (climate_ok) {
        float t, h;
        if (climate_sensor_read(&t, &h) == ESP_OK) {
            temperature = t + CLIMATE_TEMP_OFFSET_C;
            humidity    = h + CLIMATE_HUM_OFFSET_PCT;

            if (humidity < 0.0f)   humidity = 0.0f;
            if (humidity > 100.0f) humidity = 100.0f;
            
            has_climate = true;
        }
    }

    if (light_ok) {
        float l;
        if (light_sensor_read_lux(&l) == ESP_OK) {
            lux       = l;
            has_light = true;
        }
    }
}

bool ambient_get_temperature(float *celsius)
{
    if (has_climate && celsius) *celsius = temperature;
    return has_climate;
}

bool ambient_get_humidity(float *percent)
{
    if (has_climate && percent) *percent = humidity;
    return has_climate;
}

bool ambient_get_lux(float *out)
{
    if (has_light && out) *out = lux;
    return has_light;
}
