/**
 * @file climate_sensor_dht11.c
 *
 * @brief Driver del DHT11 usando el periférico RMT en recepción.
 *
 * Protocolo DHT11 (un solo hilo, open-drain):
 *   1. El host baja la línea ≥ 18 ms (señal de inicio) y la suelta.
 *   2. El DHT11 responde: 80 µs bajo + 80 µs alto.
 *   3. Envía 40 bits. Cada bit = ~50 µs bajo + pulso alto:
 *        ~26–28 µs → 0      ~70 µs → 1
 *   4. Datos: [hum_ent][hum_dec][temp_ent][temp_dec][checksum]
 *
 * ¿Por qué RMT y no "bit-banging"? Leer los pulsos a mano obliga a
 * desactivar interrupciones ~5 ms, y eso puede hacer perder paquetes
 * a la radio Zigbee. El RMT mide los pulsos por hardware.
 */
#include "climate_sensor.h"
#include "hardware_config.h"

#if CLIMATE_SENSOR_MODEL == CLIMATE_SENSOR_DHT11

#include <stdint.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "driver/rmt_rx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "CLIMATE_DHT11";

#define DHT11_START_LOW_MS     25      // señal de inicio (mínimo 18 ms)
#define DHT11_BIT_ONE_MIN_US   45      // pulso alto > 45 µs → bit 1
#define DHT11_MAX_HIGH_US      120     // pulsos más largos no son bits (reposo)
#define DHT11_TIMEOUT_MS       50      // la trama completa dura ~5 ms
#define DHT11_RX_SYMBOLS       64

static rmt_channel_handle_t rx_chan  = NULL;
static QueueHandle_t        rx_queue = NULL;
static rmt_symbol_word_t    rx_symbols[DHT11_RX_SYMBOLS];

/* Se ejecuta en la ISR del RMT al terminar la recepción */
static bool rx_done_cb(rmt_channel_handle_t chan, const rmt_rx_done_event_data_t *edata, void *user_ctx)
{
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR((QueueHandle_t)user_ctx, edata, &woken);
    return woken == pdTRUE;
}

/*---------------------------------------------------------------
 * Inicialización
 *-------------------------------------------------------------*/
esp_err_t climate_sensor_init(void)
{
    /* Línea open-drain con pull-up: el host solo "tira hacia abajo" */
    gpio_config_t io_cfg = {
        .pin_bit_mask = 1ULL << DHT11_GPIO,
        .mode         = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&io_cfg), TAG, "GPIO");
    gpio_set_level(DHT11_GPIO, 1);                       // línea libre

    rmt_rx_channel_config_t rx_cfg = {
        .gpio_num          = DHT11_GPIO,
        .clk_src           = RMT_CLK_SRC_DEFAULT,
        .resolution_hz     = 1000000,                    // 1 tick = 1 µs
        .mem_block_symbols = 48,                         // memoria de 1 canal en el H2 (trama ≈ 43)
    };
    ESP_RETURN_ON_ERROR(rmt_new_rx_channel(&rx_cfg, &rx_chan), TAG, "Canal RMT RX");

    rx_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
    ESP_RETURN_ON_FALSE(rx_queue, ESP_ERR_NO_MEM, TAG, "Cola RMT");

    rmt_rx_event_callbacks_t cbs = { .on_recv_done = rx_done_cb };
    ESP_RETURN_ON_ERROR(rmt_rx_register_event_callbacks(rx_chan, &cbs, rx_queue), TAG, "Callbacks RMT");
    ESP_RETURN_ON_ERROR(rmt_enable(rx_chan), TAG, "Activar RMT");

    ESP_LOGI(TAG, "Inicializado (GPIO %d)", DHT11_GPIO);
    return ESP_OK;
}

/*---------------------------------------------------------------
 * Lectura
 *-------------------------------------------------------------*/
esp_err_t climate_sensor_read(float *temperature_c, float *humidity_pct)
{
    ESP_RETURN_ON_FALSE(temperature_c && humidity_pct, ESP_ERR_INVALID_ARG, TAG, "Argumentos");
    ESP_RETURN_ON_FALSE(rx_chan, ESP_ERR_INVALID_STATE, TAG, "No inicializado");

    rmt_receive_config_t rcv_cfg = {
        .signal_range_min_ns = 1000,       // ignora glitches < 1 µs
        .signal_range_max_ns = 200000,     // 200 µs sin flancos = fin de trama
    };

    xQueueReset(rx_queue);

    /* 1. Señal de inicio */
    gpio_set_level(DHT11_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(DHT11_START_LOW_MS + 1)); // > 18 ms

    /* 2. Empezar a escuchar y soltar la línea */
    ESP_RETURN_ON_ERROR(rmt_receive(rx_chan, rx_symbols, sizeof(rx_symbols), &rcv_cfg),
                        TAG, "rmt_receive");
    gpio_set_level(DHT11_GPIO, 1);

    rmt_rx_done_event_data_t ev;
    if (xQueueReceive(rx_queue, &ev, pdMS_TO_TICKS(DHT11_TIMEOUT_MS)) != pdTRUE) {
        rmt_disable(rx_chan);                            // aborta la recepción pendiente
        rmt_enable(rx_chan);
        ESP_LOGW(TAG, "El DHT11 no responde");
        return ESP_ERR_TIMEOUT;

        //DEBUG
        ESP_LOGI(TAG, "%u símbolos", (unsigned)ev.num_symbols);
        for (size_t i = 0; i < ev.num_symbols && i < 6; i++) {
            const rmt_symbol_word_t *s = &ev.received_symbols[i];
            ESP_LOGI(TAG, "  [%u] %d:%u  %d:%u", (unsigned)i,
                 s->level0, s->duration0, s->level1, s->duration1);
        }
    }

    /* 3. Duraciones de los pulsos ALTOS (son los que codifican los bits) */
    uint16_t highs[DHT11_RX_SYMBOLS * 2];
    int n = 0;

    for (size_t i = 0; i < ev.num_symbols; i++) {
        const rmt_symbol_word_t *s = &ev.received_symbols[i];

        if (s->level0 == 1 && s->duration0 > 0 && s->duration0 < DHT11_MAX_HIGH_US) {
            highs[n++] = s->duration0;
        }
        if (s->level1 == 1 && s->duration1 > 0 && s->duration1 < DHT11_MAX_HIGH_US) {
            highs[n++] = s->duration1;
        }
    }

    ESP_RETURN_ON_FALSE(n >= 40, ESP_ERR_INVALID_RESPONSE, TAG,
                        "Trama incompleta (%d pulsos)", n);

    /* 4. Los 40 últimos pulsos altos son los bits de datos */
    uint8_t data[5] = { 0 };
    const uint16_t *bits = &highs[n - 40];

    for (int i = 0; i < 40; i++) {
        data[i / 8] <<= 1;
        if (bits[i] > DHT11_BIT_ONE_MIN_US) {
            data[i / 8] |= 1;
        }
    }

    uint8_t checksum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    ESP_RETURN_ON_FALSE(checksum == data[4], ESP_ERR_INVALID_CRC, TAG,
                        "Checksum incorrecto (%02x != %02x)", checksum, data[4]);

    float hum  = data[0] + data[1] * 0.1f;
    float temp = data[2] + (data[3] & 0x7F) * 0.1f;
    if (data[3] & 0x80) {
        temp = -temp;                                    // DHT11 recientes: bit 7 = negativo
    }

    ESP_RETURN_ON_FALSE(hum <= 100.0f && temp > -40.0f && temp < 80.0f,
                        ESP_ERR_INVALID_RESPONSE, TAG, "Valores fuera de rango");

#if CLIMATE_SENSOR_LOG_RAW
    ESP_LOGI(TAG, "%.1f °C  %.1f %%", temp, hum);
#endif

    *temperature_c = temp;
    *humidity_pct  = hum;
    return ESP_OK;
}

#endif /* CLIMATE_SENSOR_MODEL == CLIMATE_SENSOR_DHT11 */
