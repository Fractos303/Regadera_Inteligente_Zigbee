#pragma once

#include "esp_err.h"

esp_err_t servo_init(void);

esp_err_t servo_set_angle(float angle);

esp_err_t servo_open(void);

esp_err_t servo_close(void);

float servo_get_angle(void);

esp_err_t servo_deinit(void);