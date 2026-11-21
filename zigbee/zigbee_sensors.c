#include "zigbee_sensors.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_log.h"
#include "esp_zigbee_core.h"

#include "zigbee_config.h"
#include "water_level.h"
#include "ambient.h"

static const char *TAG = "ZB_SENSORS";

#define SENSORS_UPDATE_MS   10000   // cada 10 s

/*---------------------------------------------------------------
 * Conversión a los formatos ZCL
 *-------------------------------------------------------------*/

/* Temperature Measurement: int16 en centésimas de °C */
static int16_t zcl_temperature(float celsius)
{
    return (int16_t)lroundf(celsius * 100.0f);
}

/* Relative Humidity: uint16 en centésimas de % */
static uint16_t zcl_humidity(float percent)
{
    return (uint16_t)lroundf(percent * 100.0f);
}

/* Illuminance: uint16 = 10000·log10(lux) + 1   (0 = demasiado oscuro para medir) */
static uint16_t zcl_illuminance(float lux)
{
    if (lux < 1.0f) {
        return 0;
    }
    float v = 10000.0f * log10f(lux) + 1.0f;
    return (v > 65534.0f) ? 0xFFFE : (uint16_t)lroundf(v);
}

/*---------------------------------------------------------------
 * Escritura de atributos
 *-------------------------------------------------------------*/
static void set_attr(uint16_t cluster_id, uint16_t attr_id, void *value)
{
    esp_zb_zcl_status_t st = esp_zb_zcl_set_attribute_val(
        HA_ESP_VALVE_ENDPOINT, cluster_id, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        attr_id, value, false);

    if (st != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGW(TAG, "Cluster 0x%04x attr 0x%04x: status 0x%x", cluster_id, attr_id, st);
    }
}

/*---------------------------------------------------------------
 * Ciclo periódico (se ejecuta en la tarea Zigbee → sin lock)
 *-------------------------------------------------------------*/
static void sensors_update_cb(uint8_t param)
{
    /* Nivel de agua (Analog Input) */
    float level = water_level_update();
    set_attr(ESP_ZB_ZCL_CLUSTER_ID_ANALOG_INPUT,
             ESP_ZB_ZCL_ATTR_ANALOG_INPUT_PRESENT_VALUE_ID, &level);

    /* Ambiente */
    ambient_update();

    float t, h, l;

    if (ambient_get_temperature(&t)) {
        int16_t v = zcl_temperature(t);
        set_attr(ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
                 ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, &v);
    }

    if (ambient_get_humidity(&h)) {
        uint16_t v = zcl_humidity(h);
        set_attr(ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
                 ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID, &v);
    }

    if (ambient_get_lux(&l)) {
        uint16_t v = zcl_illuminance(l);
        set_attr(ESP_ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT,
                 ESP_ZB_ZCL_ATTR_ILLUMINANCE_MEASUREMENT_MEASURED_VALUE_ID, &v);
    }

    ESP_LOGI(TAG, "Agua %.1f %% | %.1f °C | %.0f %% HR | %.0f lx",
             level,
             ambient_get_temperature(&t) ? t : NAN,
             ambient_get_humidity(&h)    ? h : NAN,
             ambient_get_lux(&l)         ? l : NAN);

    esp_zb_scheduler_alarm(sensors_update_cb, 0, SENSORS_UPDATE_MS);
}

void zigbee_sensors_start(void)
{
    static bool started = false;
    if (started) {
        return;
    }
    started = true;

    esp_zb_scheduler_alarm(sensors_update_cb, 0, SENSORS_UPDATE_MS);
    ESP_LOGI(TAG, "Actualización de sensores cada %d ms", SENSORS_UPDATE_MS);
}
