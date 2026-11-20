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

/* ============================
 * Sensor de nivel de agua
 * ============================ */

#define WATER_SENSOR_KY059              1
#define WATER_SENSOR_GROVE_I2C          2
#define WATER_SENSOR_MODEL              WATER_SENSOR_KY059

#define WATER_SENSOR_ADC_GPIO           GPIO_NUM_4    // ADC1 (en el H2 solo GPIO1–5 tienen ADC)
#define WATER_SENSOR_POWER_GPIO         GPIO_NUM_NC   // NC = alimentado fijo a 3V3
#define WATER_SENSOR_POWER_SETTLE_MS    10

#define WATER_SENSOR_SAMPLES            16
#define WATER_SENSOR_MV_EMPTY           0             //TODO: calibrar
#define WATER_SENSOR_MV_FULL            1500          //TODO: calibrar
#define WATER_SENSOR_LOG_RAW            1             // 1 mientras calibras

#define WATER_SENSOR_I2C_PORT           I2C_NUM_0
#define WATER_SENSOR_I2C_SDA_GPIO       GPIO_NUM_10
#define WATER_SENSOR_I2C_SCL_GPIO       GPIO_NUM_11
#define WATER_SENSOR_I2C_FREQ_HZ        100000
#define WATER_SENSOR_GROVE_THRESHOLD    100

