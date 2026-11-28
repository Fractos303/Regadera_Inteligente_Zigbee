#pragma once

#include "driver/gpio.h"

/* ============================
 * Status LED
 * ============================ */

#define STATUS_LED_GPIO            GPIO_NUM_8
#define STATUS_LED_NUM_PIXELS      1

/* ============================
 * Bomba peristáltica (DRV8833, canal A)
 * ============================ */
#define PUMP_DRV8833            1
#define PUMP_MODEL              PUMP_DRV8833
#define PUMP_AIN1_GPIO          GPIO_NUM_10     // PWM
#define PUMP_AIN2_GPIO          GPIO_NUM_11
#define PUMP_NSLEEP_GPIO        GPIO_NUM_12
#define PUMP_NFAULT_GPIO        GPIO_NUM_0      // GPIO_NUM_NC si no se conecta
#define PUMP_PWM_FREQ_HZ        20000           // inaudible
#define PUMP_SOFT_START_MS      500
#define PUMP_WAKE_US            1000            // despertar del DRV8833

/* ============================
 * Sensor de nivel de agua
 * ============================ */

#define WATER_SENSOR_KY059          1
#define WATER_SENSOR_GROVE_I2C      2
#define WATER_SENSOR_HX711          3
#define WATER_SENSOR_MODEL          WATER_SENSOR_HX711

#define HX711_DOUT_GPIO             GPIO_NUM_11
#define HX711_SCK_GPIO              GPIO_NUM_12
#define HX711_TARE_RAW              -682670             //TODO calibrar (paso 2)
#define HX711_COUNTS_PER_GRAM       -408.7f             //TODO calibrar (paso 3)
#define WATER_TANK_CAPACITY_G       1000.0f             // 1 L de agua = 1000 g
#define WATER_SENSOR_LOG_RAW        1   

//#define WATER_SENSOR_ADC_GPIO           GPIO_NUM_4    // ADC1 (en el H2 solo GPIO1–5 tienen ADC)
//#define WATER_SENSOR_POWER_GPIO         GPIO_NUM_NC   // NC = alimentado fijo a 3V3
//#define WATER_SENSOR_POWER_SETTLE_MS    10
//
//#define WATER_SENSOR_SAMPLES            16
//#define WATER_SENSOR_MV_EMPTY           280             //TODO: calibrar
//#define WATER_SENSOR_MV_FULL            550             //TODO: calibrar
//#define WATER_SENSOR_LOG_RAW            1               // 1 mientras calibras
//
//#define WATER_SENSOR_I2C_PORT           I2C_NUM_0
//#define WATER_SENSOR_I2C_SDA_GPIO       GPIO_NUM_10
//#define WATER_SENSOR_I2C_SCL_GPIO       GPIO_NUM_11
//#define WATER_SENSOR_I2C_FREQ_HZ        100000
//#define WATER_SENSOR_GROVE_THRESHOLD    100

/* ============================
 * Temperatura / humedad
 * ============================ */
#define CLIMATE_SENSOR_DHT11        1
#define CLIMATE_SENSOR_MODEL        CLIMATE_SENSOR_DHT11
#define DHT11_GPIO                  GPIO_NUM_22
#define CLIMATE_SENSOR_LOG_RAW      1

#define CLIMATE_TEMP_OFFSET_C      -1.8f    // se suma a la lectura (°C)
#define CLIMATE_HUM_OFFSET_PCT      0.0f    // se suma a la lectura (%)

/* ============================
 * Luz (LDR)
 * ============================ */
#define LIGHT_SENSOR_LDR            1
#define LIGHT_SENSOR_MODEL          LIGHT_SENSOR_LDR
#define LIGHT_SENSOR_ADC_GPIO       GPIO_NUM_1      // ADC1 (en el H2 solo GPIO1–5 tienen ADC)
#define LDR_ON_HIGH_SIDE            1               // LDR entre 3V3 y GPIO5
#define LDR_R_FIXED                 10000           // resistencia fija del módulo (Ω), entre GPIO5 y GND
#define LDR_R_REF                   15000           // Ω del LDR a LDR_LUX_REF lux (GL5528 típico)
#define LDR_LUX_REF                 10.0f
#define LDR_GAMMA                   0.7f
#define LIGHT_SENSOR_LOG_RAW        1

/* ============================
 * Humedad del suelo
 * ============================ */
#define SOIL_SENSOR_CAPACITIVE        1
#define SOIL_SENSOR_MODEL             SOIL_SENSOR_CAPACITIVE
#define SOIL_SENSOR_ADC_GPIO          GPIO_NUM_4
#define SOIL_SENSOR_POWER_GPIO        GPIO_NUM_NC     // alimentado fijo a 3V3
#define SOIL_SENSOR_POWER_SETTLE_MS   200
#define SOIL_SENSOR_SAMPLES           16
#define SOIL_MV_DRY                   2700            //TODO ← calibrar: en el aire
#define SOIL_MV_WET                   1200            //TODO ← calibrar: en agua hasta la línea
#define SOIL_SENSOR_LOG_RAW           1