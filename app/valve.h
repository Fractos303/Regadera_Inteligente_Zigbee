#pragma once

#include <stdbool.h>

typedef enum
{
    VALVE_CLOSED = 0,
    VALVE_OPEN
} valve_state_t;

void valve_init(void);

void valve_set_state(valve_state_t state);

valve_state_t valve_get_state(void);