#include "zigbee_endpoint.h"

#include "esp_zigbee_cluster.h"
#include "esp_zigbee_endpoint.h"
#include "zigbee_config.h"

/*
 * Estructura Zigbee del dispositivo (lo que ZHA lee al emparejar).
 * Cambiar algo aquí obliga a volver a emparejar.
 *
 *   Endpoint 10 (HA_ESP_VALVE_ENDPOINT)
 *     Basic, Identify, Groups, Scenes
 *     On/Off                → riego (bomba)
 *     Analog Input          → nivel de agua (%)
 *     Temperature           → temperatura del aire
 *     Relative Humidity     → humedad del aire
 *     Illuminance           → luz
 *
 *   Endpoint 11 (HA_ESP_SOIL_ENDPOINT)
 *     Analog Input          → humedad del suelo (%)
 *
 * El endpoint 11 existe porque un endpoint solo admite un clúster de cada
 * tipo, y el 10 ya usa Analog Input. El clúster Soil Moisture (0x0408) no
 * está soportado por la librería Zigbee interna: al escribirlo provoca un assert.
 */

/*---------------------------------------------------------------
 * Endpoint 10: dispositivo principal
 *-------------------------------------------------------------*/
static void add_main_endpoint(esp_zb_ep_list_t *ep_list)
{
    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();

    /* Basic: fabricante y modelo */
    esp_zb_attribute_list_t *basic = esp_zb_basic_cluster_create(NULL);
    esp_zb_basic_cluster_add_attr(basic, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, ESP_MANUFACTURER_NAME);
    esp_zb_basic_cluster_add_attr(basic, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID,  ESP_MODEL_IDENTIFIER);
    esp_zb_cluster_list_add_basic_cluster(cluster_list, basic, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    /* Identify, Groups, Scenes */
    esp_zb_cluster_list_add_identify_cluster(cluster_list, esp_zb_identify_cluster_create(NULL), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_groups_cluster(cluster_list,   esp_zb_groups_cluster_create(NULL),   ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_scenes_cluster(cluster_list,   esp_zb_scenes_cluster_create(NULL),   ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    /* On/Off → riego */
    esp_zb_cluster_list_add_on_off_cluster(cluster_list, esp_zb_on_off_cluster_create(NULL), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    /* Analog Input → nivel de agua */
    esp_zb_analog_input_cluster_cfg_t water_cfg = {
        .out_of_service = false,
        .present_value  = 0.0f,
        .status_flags   = ESP_ZB_ZCL_ANALOG_INPUT_STATUS_FLAG_NORMAL,
    };
    esp_zb_attribute_list_t *water_ai = esp_zb_analog_input_cluster_create(&water_cfg);

    /* ZHA exige 'description' + ('engineering_units' o 'application_type') para crear la entidad */
    static char     water_description[] = "\x0A""Nivel agua";   // ZCL string: 1er byte = longitud (10)
    static uint16_t water_units         = 98;                   // BACnet 98 = porcentaje → "%"
    static float    water_resolution    = 1.0f;                 // 0 decimales en HA
    static float    water_min = 0.0f, water_max = 100.0f;

    esp_zb_analog_input_cluster_add_attr(water_ai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_DESCRIPTION_ID,       water_description);
    esp_zb_analog_input_cluster_add_attr(water_ai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_ENGINEERING_UNITS_ID, &water_units);
    esp_zb_analog_input_cluster_add_attr(water_ai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_RESOLUTION_ID,        &water_resolution);
    esp_zb_analog_input_cluster_add_attr(water_ai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_MIN_PRESENT_VALUE_ID, &water_min);
    esp_zb_analog_input_cluster_add_attr(water_ai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_MAX_PRESENT_VALUE_ID, &water_max);

    esp_zb_cluster_list_add_analog_input_cluster(cluster_list, water_ai, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    /* Temperatura, humedad del aire e iluminancia (valores por defecto del SDK) */
    esp_zb_cluster_list_add_temperature_meas_cluster(cluster_list, esp_zb_temperature_meas_cluster_create(NULL), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_humidity_meas_cluster(cluster_list,    esp_zb_humidity_meas_cluster_create(NULL),    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_illuminance_meas_cluster(cluster_list, esp_zb_illuminance_meas_cluster_create(NULL), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    /* Añadir el endpoint, con TODOS sus clústeres ya en la lista */
    esp_zb_endpoint_config_t endpoint = {
        .endpoint           = HA_ESP_VALVE_ENDPOINT,
        .app_profile_id     = HA_PROFILE_ID,
        .app_device_id      = HA_DEVICE_ID,
        .app_device_version = HA_DEVICE_VERSION,
    };
    esp_zb_ep_list_add_ep(ep_list, cluster_list, endpoint);
}

/*---------------------------------------------------------------
 * Endpoint 11: humedad del suelo
 *-------------------------------------------------------------*/
static void add_soil_endpoint(esp_zb_ep_list_t *ep_list)
{
    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();

    esp_zb_analog_input_cluster_cfg_t soil_cfg = {
        .out_of_service = false,
        .present_value  = 0.0f,
        .status_flags   = ESP_ZB_ZCL_ANALOG_INPUT_STATUS_FLAG_NORMAL,
    };
    esp_zb_attribute_list_t *soil_ai = esp_zb_analog_input_cluster_create(&soil_cfg);

    static char     soil_description[] = "\x0D""Humedad suelo";   // 13 caracteres
    static uint16_t soil_units         = 98;                       // BACnet: porcentaje
    static float    soil_resolution    = 1.0f;
    static float    soil_min = 0.0f, soil_max = 100.0f;

    esp_zb_analog_input_cluster_add_attr(soil_ai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_DESCRIPTION_ID,       soil_description);
    esp_zb_analog_input_cluster_add_attr(soil_ai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_ENGINEERING_UNITS_ID, &soil_units);
    esp_zb_analog_input_cluster_add_attr(soil_ai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_RESOLUTION_ID,        &soil_resolution);
    esp_zb_analog_input_cluster_add_attr(soil_ai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_MIN_PRESENT_VALUE_ID, &soil_min);
    esp_zb_analog_input_cluster_add_attr(soil_ai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_MAX_PRESENT_VALUE_ID, &soil_max);

    esp_zb_cluster_list_add_analog_input_cluster(cluster_list, soil_ai, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    esp_zb_endpoint_config_t endpoint = {
        .endpoint           = HA_ESP_SOIL_ENDPOINT,
        .app_profile_id     = HA_PROFILE_ID,
        .app_device_id      = ESP_ZB_HA_SIMPLE_SENSOR_DEVICE_ID,   // si no compila: 0x000C
        .app_device_version = HA_DEVICE_VERSION,
    };
    esp_zb_ep_list_add_ep(ep_list, cluster_list, endpoint);
}

/*---------------------------------------------------------------
 * API
 *-------------------------------------------------------------*/
esp_zb_ep_list_t *zigbee_create_endpoint(void)
{
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();

    add_main_endpoint(ep_list);
    add_soil_endpoint(ep_list);

    return ep_list;
}
