/**
 * @file water_sensor_grove_ATtiny1616.c
 *
 * @brief Driver del Grove - Water Level Sensor (10 cm) de Seeed (SKU 101020635).
 *
 * Sensor capacitivo con 20 secciones de 5 mm, leídas por dos ATtiny1616
 * que responden por I2C en dos direcciones distintas:
 *   - 0x77 → 8 secciones inferiores
 *   - 0x78 → 12 secciones superiores
 *
 * Cada sección devuelve un byte (0–255). Si supera el umbral se considera
 * sumergida. El nivel es el número de secciones sumergidas CONSECUTIVAS
 * desde abajo × 5 %. Así se ignoran gotas sueltas en secciones superiores.
 *
 * Al ser capacitivo, las pistas no tocan el agua: no hay corrosión
 * y no hace falta alimentación conmutada.
 *
 * ⚠ La zona de la electrónica (parte superior) NO es estanca.
 */
#include "water_sensor.h"
#include "hardware_config.h"

#if WATER_SENSOR_MODEL == WATER_SENSOR_GROVE_I2C

#include <stdint.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c_master.h"

static const char *TAG = "WATER_SENSOR_GROVE";

#define GROVE_ADDR_LOW           0x77    // 8 secciones inferiores
#define GROVE_ADDR_HIGH          0x78    // 12 secciones superiores

#define GROVE_SECTIONS_LOW       8
#define GROVE_SECTIONS_HIGH      12
#define GROVE_SECTIONS_TOTAL     (GROVE_SECTIONS_LOW + GROVE_SECTIONS_HIGH)   // 20 × 5 mm = 10 cm

#define GROVE_I2C_TIMEOUT_MS     100

static i2c_master_bus_handle_t bus      = NULL;
static i2c_master_dev_handle_t dev_low  = NULL;
static i2c_master_dev_handle_t dev_high = NULL;
static bool                    present  = false;

/*---------------------------------------------------------------
 * Inicialización
 *-------------------------------------------------------------*/
esp_err_t water_sensor_init(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port          = WATER_SENSOR_I2C_PORT,
        .sda_io_num        = WATER_SENSOR_I2C_SDA_GPIO,
        .scl_io_num        = WATER_SENSOR_I2C_SCL_GPIO,
        .clk_source        = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,   // débiles: con cables largos, añadir 4,7 kΩ externas
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &bus), TAG, "No se pudo crear el bus I2C");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .scl_speed_hz    = WATER_SENSOR_I2C_FREQ_HZ,
    };

    dev_cfg.device_address = GROVE_ADDR_LOW;
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus, &dev_cfg, &dev_low), TAG, "Dispositivo 0x77");

    dev_cfg.device_address = GROVE_ADDR_HIGH;
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus, &dev_cfg, &dev_high), TAG, "Dispositivo 0x78");

    /* Si el sensor no responde NO se devuelve error: así un sensor
       desconectado no provoca un bucle de reinicios. Las lecturas
       fallarán y water_level mantendrá el último valor. */
    present = (i2c_master_probe(bus, GROVE_ADDR_LOW,  GROVE_I2C_TIMEOUT_MS) == ESP_OK)
           && (i2c_master_probe(bus, GROVE_ADDR_HIGH, GROVE_I2C_TIMEOUT_MS) == ESP_OK);

    if (present) {
        ESP_LOGI(TAG, "Inicializado (SDA=%d, SCL=%d, umbral=%d)",
                 WATER_SENSOR_I2C_SDA_GPIO, WATER_SENSOR_I2C_SCL_GPIO,
                 WATER_SENSOR_GROVE_THRESHOLD);
    } else {
        ESP_LOGW(TAG, "El sensor no responde en 0x77/0x78: revisa conexión y pull-ups");
    }

    return ESP_OK;
}

/*---------------------------------------------------------------
 * Lectura
 *-------------------------------------------------------------*/
esp_err_t water_sensor_read_percent(float *percent)
{
    ESP_RETURN_ON_FALSE(percent, ESP_ERR_INVALID_ARG,   TAG, "percent NULL");
    ESP_RETURN_ON_FALSE(bus,     ESP_ERR_INVALID_STATE, TAG, "Sensor no inicializado");

    uint8_t low[GROVE_SECTIONS_LOW];
    uint8_t high[GROVE_SECTIONS_HIGH];

    ESP_RETURN_ON_ERROR(i2c_master_receive(dev_low,  low,  sizeof(low),  GROVE_I2C_TIMEOUT_MS),
                        TAG, "Fallo leyendo 0x77");
    ESP_RETURN_ON_ERROR(i2c_master_receive(dev_high, high, sizeof(high), GROVE_I2C_TIMEOUT_MS),
                        TAG, "Fallo leyendo 0x78");

    if (!present) {
        ESP_LOGI(TAG, "Sensor detectado");
        present = true;
    }

    /* Máscara de secciones sumergidas: bits 0–7 abajo, 8–19 arriba */
    uint32_t wet_mask = 0;

    for (int i = 0; i < GROVE_SECTIONS_LOW; i++) {
        if (low[i] > WATER_SENSOR_GROVE_THRESHOLD) {
            wet_mask |= (1u << i);
        }
    }
    for (int i = 0; i < GROVE_SECTIONS_HIGH; i++) {
        if (high[i] > WATER_SENSOR_GROVE_THRESHOLD) {
            wet_mask |= (1u << (GROVE_SECTIONS_LOW + i));
        }
    }

    /* Secciones consecutivas desde abajo */
    int sections = 0;
    while (wet_mask & 1u) {
        sections++;
        wet_mask >>= 1;
    }

    float pct = (float)sections * 100.0f / (float)GROVE_SECTIONS_TOTAL;

#if WATER_SENSOR_LOG_RAW
    ESP_LOG_BUFFER_HEX(TAG, low,  sizeof(low));     // valores crudos para ajustar el umbral
    ESP_LOG_BUFFER_HEX(TAG, high, sizeof(high));
    ESP_LOGI(TAG, "%d/%d secciones -> %.0f %%", sections, GROVE_SECTIONS_TOTAL, pct);
#endif

    *percent = pct;
    return ESP_OK;
}

#endif /* WATER_SENSOR_MODEL == WATER_SENSOR_GROVE_I2C */
