#pragma once

#include "driver/gpio.h"

/* ============================
 * Status LED
 * ============================ */

#define STATUS_LED_GPIO            GPIO_NUM_8
#define STATUS_LED_NUM_PIXELS      1

/* ============================
 * Servo
 * ============================ */

#define SERVO_GPIO                 GPIO_NUM_12

#define SERVO_OPEN_ANGLE           90.0f
#define SERVO_CLOSE_ANGLE          0.0f