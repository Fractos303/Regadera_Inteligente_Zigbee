/**
 * @file hx711.h
 *
 * @brief Driver de bajo nivel del conversor HX711 (24 bits, para células de carga).
 *
 * Protocolo (2 hilos, no es I2C):
 *   - DOUT pasa a 0 cuando hay un dato nuevo listo (10 por segundo con RATE = GND).
 *   - El host da 24 pulsos en SCK y lee un bit por pulso (MSB primero).
 *   - 1, 2 o 3 pulsos extra eligen canal y ganancia para la SIGUIENTE conversión.
 *   - SCK a nivel alto más de 60 µs → el HX711 se apaga (modo bajo consumo).
 *
 * Es genérico: no sabe nada de depósitos ni de gramos. Eso lo hace
 * water_sensor_hx711.c, que lo usa.
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Pulsos extra tras los 24 bits = canal y ganancia de la siguiente lectura */
typedef enum {
    HX711_GAIN_A_128 = 1,       // canal A, ganancia 128 (lo normal con célula de carga)
    HX711_GAIN_B_32  = 2,       // canal B, ganancia 32
    HX711_GAIN_A_64  = 3,       // canal A, ganancia 64
} hx711_gain_t;

typedef struct {
    gpio_num_t   dout;
    gpio_num_t   sck;
    hx711_gain_t gain;
} hx711_t;

esp_err_t hx711_init(const hx711_t *dev);

/**
 * @brief Espera a que haya un dato y lo lee.
 *
 * @param[out] raw        Valor con signo (24 bits extendidos a 32).
 * @param      timeout_ms Tiempo máximo de espera del dato.
 */
esp_err_t hx711_read_raw(const hx711_t *dev, int32_t *raw, uint32_t timeout_ms);

/** Apaga el HX711 (~1 µA). Útil en el futuro con pilas. */
void hx711_power_down(const hx711_t *dev);

/** Lo enciende. La primera lectura tarda ~400 ms en estabilizarse. */
void hx711_power_up(const hx711_t *dev);

#ifdef __cplusplus
}
#endif
