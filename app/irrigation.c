#include "irrigation.h"

#include <stdint.h>

#include "esp_log.h"
#include "esp_check.h"
#include "esp_timer.h"

#include "pump.h"
#include "water_level.h"
#include "device_feedback.h"

static const char *TAG = "IRRIGATION";

/*---------------------------------------------------------------
 * Parámetros
 *-------------------------------------------------------------*/
#define IRRIGATION_PUMP_SPEED_PCT    100    // velocidad de la bomba al regar
#define IRRIGATION_MAX_RUN_S         120    // seguridad: tiempo máximo seguido (0 = sin límite)

/* Bloqueo por depósito vacío.
 * 0 mientras se usa el KY-059 en la mesa (en seco marca 0 % y bloquearía siempre).
 * 1 cuando la báscula HX711 esté integrada y calibrada. */
#define IRRIGATION_TANK_INTERLOCK    0

/*---------------------------------------------------------------
 * Estado
 *-------------------------------------------------------------*/
static bool                  running  = false;
static int64_t               start_us = 0;
static irrigation_state_cb_t state_cb = NULL;

static void notify(bool now_running, irrigation_reason_t reason)
{
    if (state_cb) {
        state_cb(now_running, reason);
    }
}

static void stop_pump(void)
{
    pump_stop();
    running = false;
}

static bool tank_is_empty(void)
{
#if IRRIGATION_TANK_INTERLOCK
    return water_level_is_empty();
#else
    return false;
#endif
}

/*---------------------------------------------------------------
 * API
 *-------------------------------------------------------------*/
esp_err_t irrigation_init(void)
{
    ESP_RETURN_ON_ERROR(pump_init(), TAG, "Bomba");

    ESP_LOGI(TAG, "Irrigation initialized (máx %d s, bloqueo por depósito %s)",
             IRRIGATION_MAX_RUN_S, IRRIGATION_TANK_INTERLOCK ? "ACTIVO" : "desactivado");
    return ESP_OK;
}

void irrigation_register_state_cb(irrigation_state_cb_t cb)
{
    state_cb = cb;
}

esp_err_t irrigation_request(bool on)
{
    if (!on) {
        if (running) {
            ESP_LOGI(TAG, "Riego OFF");
        }
        stop_pump();
        return ESP_OK;
    }

    if (running) {
        return ESP_OK;                              // ya está regando
    }

    if (tank_is_empty()) {
        ESP_LOGW(TAG, "Orden rechazada: depósito vacío");
        device_feedback_tank_empty();
        notify(false, IRRIGATION_REASON_TANK_EMPTY);   // devolver el interruptor a OFF
        return ESP_ERR_NOT_ALLOWED;
    }

    if (pump_has_fault()) {
        ESP_LOGE(TAG, "Orden rechazada: fallo en el driver de la bomba");
        device_feedback_error();
        notify(false, IRRIGATION_REASON_FAULT);
        return ESP_ERR_NOT_ALLOWED;
    }

    ESP_RETURN_ON_ERROR(pump_start(IRRIGATION_PUMP_SPEED_PCT), TAG, "Arrancar bomba");

    running  = true;
    start_us = esp_timer_get_time();

    ESP_LOGI(TAG, "Riego ON");
    return ESP_OK;
}

void irrigation_tick(void)
{
    if (!running) {
        return;
    }

    if (tank_is_empty()) {
        ESP_LOGW(TAG, "Depósito vacío: parando el riego");
        stop_pump();
        device_feedback_tank_empty();
        notify(false, IRRIGATION_REASON_TANK_EMPTY);
        return;
    }

    if (pump_has_fault()) {
        ESP_LOGE(TAG, "Fallo del driver: parando el riego");
        stop_pump();
        device_feedback_error();
        notify(false, IRRIGATION_REASON_FAULT);
        return;
    }

    if (IRRIGATION_MAX_RUN_S > 0 &&
        (esp_timer_get_time() - start_us) > (int64_t)IRRIGATION_MAX_RUN_S * 1000000LL) {
        ESP_LOGW(TAG, "Tiempo máximo de riego alcanzado (%d s): parando", IRRIGATION_MAX_RUN_S);
        stop_pump();
        notify(false, IRRIGATION_REASON_TIMEOUT);
    }
}

bool irrigation_is_running(void)
{
    return running;
}
