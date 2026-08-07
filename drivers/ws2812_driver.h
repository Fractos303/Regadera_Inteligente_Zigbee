#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* light intensity level */
#define LIGHT_DEFAULT_ON  1
#define LIGHT_DEFAULT_OFF 0

/* LED strip configuration */
#define CONFIG_EXAMPLE_STRIP_LED_GPIO   8  //LED BUILTIN
#define CONFIG_EXAMPLE_STRIP_LED_NUMBER 1

esp_err_t ws2812_driver_init(gpio_num_t gpio, uint8_t num_leds);

esp_err_t ws2812_driver_set_rgb(uint8_t led, uint8_t red, uint8_t green, uint8_t blue);

esp_err_t ws2812_driver_clear(void);

#ifdef __cplusplus
} // extern "C"
#endif