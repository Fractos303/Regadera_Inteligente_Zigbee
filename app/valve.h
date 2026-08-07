#pragma once

#include "esp_err.h"

typedef enum
{
    VALVE_CLOSED = 0,
    VALVE_OPEN

} valve_state_t;

esp_err_t valve_init(void);

esp_err_t valve_set_state(valve_state_t state);

valve_state_t valve_get_state(void);