#include "zigbee_actuators.h"

#include <stdint.h>

#include "esp_log.h"
#include "esp_zigbee_core.h"

#include "zigbee_config.h"
#include "irrigation.h"

static const char *TAG = "ZB_ACTUATORS";

#define ACTUATORS_TICK_MS     1000    // comprobaciones de seguridad cada 1 s
#define ONOFF_SYNC_DELAY_MS   50      // margen para no pisar la escritura en curso

/*---------------------------------------------------------------
 * Sincronizar el atributo On/Off con el estado real
 *-------------------------------------------------------------*/
static void onoff_sync_cb(uint8_t on)
{
    bool value = (on != 0);

    esp_zb_zcl_status_t st = esp_zb_zcl_set_attribute_val(
        HA_ESP_VALVE_ENDPOINT,
        ESP_ZB_ZCL_CLUSTER_ID_ON_OFF,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
        &value,
        false);

    if (st != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGW(TAG, "No se pudo actualizar On/Off (status 0x%x)", st);
    } else {
        ESP_LOGI(TAG, "Interruptor de HA -> %s", value ? "ON" : "OFF");
    }
}

/* Llamado por app/irrigation cuando cambia el estado por una causa interna.
 * Siempre ocurre dentro de la tarea Zigbee (orden de HA o tick), así que
 * se puede usar el planificador Zigbee sin lock. Se aplaza unos ms porque,
 * si viene de una orden de HA, el stack aún está procesando esa escritura. */
static void irrigation_state_changed(bool running, irrigation_reason_t reason)
{
    static const char *reasons[] = { "depósito vacío", "tiempo máximo", "fallo" };
    ESP_LOGI(TAG, "Riego %s por: %s", running ? "ON" : "OFF", reasons[reason]);

    esp_zb_scheduler_alarm(onoff_sync_cb, running ? 1 : 0, ONOFF_SYNC_DELAY_MS);
}

/*---------------------------------------------------------------
 * Ciclo de seguridad
 *-------------------------------------------------------------*/
static void actuators_tick_cb(uint8_t param)
{
    irrigation_tick();
    esp_zb_scheduler_alarm(actuators_tick_cb, 0, ACTUATORS_TICK_MS);
}

void zigbee_actuators_start(void)
{
    static bool started = false;
    if (started) {
        return;
    }
    started = true;

    irrigation_register_state_cb(irrigation_state_changed);
    esp_zb_scheduler_alarm(actuators_tick_cb, 0, ACTUATORS_TICK_MS);

    ESP_LOGI(TAG, "Actuadores listos (seguridad cada %d ms)", ACTUATORS_TICK_MS);
}

/*---------------------------------------------------------------
 * Órdenes de Home Assistant
 *-------------------------------------------------------------*/
void zigbee_actuators_handle_on_off(bool on)
{
    ESP_LOGI(TAG, "Orden de HA: riego %s", on ? "ON" : "OFF");

    if (irrigation_request(on) == ESP_ERR_NOT_ALLOWED) {
        ESP_LOGW(TAG, "Orden rechazada por seguridad");
        /* irrigation ya ha avisado por el callback → el interruptor volverá a OFF */
    }
}
