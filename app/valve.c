#include "valve.h"
#include "esp_log.h"

static const char *TAG = "VALVE";

static valve_state_t state = VALVE_CLOSED;

void valve_init(void)
{
    ESP_LOGI(TAG,"Valve initialized");
}

void valve_set_state(valve_state_t new_state)
{
    state = new_state;

    ESP_LOGI(TAG,
             "Valve %s",
             state == VALVE_OPEN ?
             "OPEN" :
             "CLOSED");
}

valve_state_t valve_get_state(void)
{
    return state;
}