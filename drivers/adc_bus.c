#include "adc_bus.h"

#include "esp_log.h"
#include "esp_check.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "ADC_BUS";

/* Una entrada por unidad ADC (en el H2 solo se usa ADC_UNIT_1) */
static adc_oneshot_unit_handle_t units[2] = { NULL, NULL };

static esp_err_t get_unit(adc_unit_t unit, adc_oneshot_unit_handle_t *handle)
{
    ESP_RETURN_ON_FALSE(unit < 2, ESP_ERR_INVALID_ARG, TAG, "Unidad ADC no válida");

    if (units[unit] == NULL) {
        adc_oneshot_unit_init_cfg_t cfg = { .unit_id = unit };
        ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&cfg, &units[unit]), TAG, "No se pudo crear ADC%d", unit + 1);
        ESP_LOGI(TAG, "ADC%d inicializado", unit + 1);
    }

    *handle = units[unit];
    return ESP_OK;
}

esp_err_t adc_bus_add_channel(gpio_num_t gpio, adc_bus_channel_t *out)
{
    ESP_RETURN_ON_FALSE(out, ESP_ERR_INVALID_ARG, TAG, "out NULL");

    ESP_RETURN_ON_ERROR(adc_oneshot_io_to_channel(gpio, &out->unit, &out->channel),
                        TAG, "GPIO %d no tiene ADC", gpio);

    adc_oneshot_unit_handle_t handle;
    ESP_RETURN_ON_ERROR(get_unit(out->unit, &handle), TAG, "unidad");

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(handle, out->channel, &chan_cfg),
                        TAG, "No se pudo configurar el canal %d", out->channel);

    out->cali = NULL;
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id  = out->unit,
        .chan     = out->channel,
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &out->cali) != ESP_OK) {
        ESP_LOGW(TAG, "GPIO %d sin calibración: mV aproximados", gpio);
        out->cali = NULL;
    }
#endif

    ESP_LOGI(TAG, "GPIO %d -> ADC%d canal %d", gpio, out->unit + 1, out->channel);
    return ESP_OK;
}

esp_err_t adc_bus_read_mv(const adc_bus_channel_t *ch, int samples, int *mv)
{
    ESP_RETURN_ON_FALSE(ch && mv && samples > 0, ESP_ERR_INVALID_ARG, TAG, "Argumentos");
    ESP_RETURN_ON_FALSE(units[ch->unit], ESP_ERR_INVALID_STATE, TAG, "ADC no inicializado");

    int sum   = 0;
    int valid = 0;

    for (int i = 0; i < samples; i++) {
        int raw;
        if (adc_oneshot_read(units[ch->unit], ch->channel, &raw) != ESP_OK) {
            continue;
        }

        int v;
        if (ch->cali) {
            adc_cali_raw_to_voltage(ch->cali, raw, &v);
        } else {
            v = raw * 3300 / 4095;
        }

        sum += v;
        valid++;
    }

    ESP_RETURN_ON_FALSE(valid > 0, ESP_FAIL, TAG, "Ninguna lectura ADC válida");

    *mv = sum / valid;
    return ESP_OK;
}
