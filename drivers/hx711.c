#include "hx711.h"

#include <stdbool.h>

#include "esp_log.h"
#include "esp_check.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "HX711";

/* Sección crítica para la ráfaga de pulsos: si una interrupción dejara SCK
 * a nivel alto más de 60 µs, el HX711 se apagaría a mitad de la lectura.
 * La ráfaga entera dura ~60–100 µs, así que no afecta a la radio Zigbee. */
static portMUX_TYPE hx711_mux = portMUX_INITIALIZER_UNLOCKED;

esp_err_t hx711_init(const hx711_t *dev)
{
    ESP_RETURN_ON_FALSE(dev, ESP_ERR_INVALID_ARG, TAG, "dev NULL");

    /* SCK: salida a 0 (HX711 encendido) */
    ESP_RETURN_ON_ERROR(gpio_reset_pin(dev->sck), TAG, "SCK");
    ESP_RETURN_ON_ERROR(gpio_set_direction(dev->sck, GPIO_MODE_OUTPUT), TAG, "SCK");
    gpio_set_level(dev->sck, 0);

    /* DOUT: entrada. El pull-up hace que, si el módulo está desconectado,
     * la línea quede a 1 ("sin dato") y la lectura dé timeout en vez de basura. */
    ESP_RETURN_ON_ERROR(gpio_reset_pin(dev->dout), TAG, "DOUT");
    ESP_RETURN_ON_ERROR(gpio_set_direction(dev->dout, GPIO_MODE_INPUT), TAG, "DOUT");
    ESP_RETURN_ON_ERROR(gpio_set_pull_mode(dev->dout, GPIO_PULLUP_ONLY), TAG, "DOUT pull-up");

    ESP_LOGI(TAG, "Inicializado (DOUT=%d, SCK=%d, pulsos de ganancia=%d)",
             dev->dout, dev->sck, dev->gain);
    return ESP_OK;
}

esp_err_t hx711_read_raw(const hx711_t *dev, int32_t *raw, uint32_t timeout_ms)
{
    ESP_RETURN_ON_FALSE(dev && raw, ESP_ERR_INVALID_ARG, TAG, "Argumentos");

    /* 1. Esperar a que DOUT baje (dato listo) */
    TickType_t start = xTaskGetTickCount();
    while (gpio_get_level(dev->dout) == 1) {
        if ((xTaskGetTickCount() - start) > pdMS_TO_TICKS(timeout_ms)) {
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(1);
    }

    /* 2. Leer 24 bits + pulsos de ganancia */
    uint32_t value = 0;

    portENTER_CRITICAL(&hx711_mux);

    for (int i = 0; i < 24; i++) {
        gpio_set_level(dev->sck, 1);
        esp_rom_delay_us(1);
        value = (value << 1) | (uint32_t)gpio_get_level(dev->dout);
        gpio_set_level(dev->sck, 0);
        esp_rom_delay_us(1);
    }

    for (int i = 0; i < (int)dev->gain; i++) {
        gpio_set_level(dev->sck, 1);
        esp_rom_delay_us(1);
        gpio_set_level(dev->sck, 0);
        esp_rom_delay_us(1);
    }

    portEXIT_CRITICAL(&hx711_mux);

    /* 3. Complemento a 2 de 24 bits → int32 */
    if (value & 0x800000) {
        value |= 0xFF000000;
    }

    *raw = (int32_t)value;
    return ESP_OK;
}

void hx711_power_down(const hx711_t *dev)
{
    gpio_set_level(dev->sck, 0);
    gpio_set_level(dev->sck, 1);
    esp_rom_delay_us(100);              // > 60 µs a nivel alto → apagado
}

void hx711_power_up(const hx711_t *dev)
{
    gpio_set_level(dev->sck, 0);        // al encender vuelve a canal A / ganancia 128
}
