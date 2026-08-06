#include "zigbee_endpoint.h"

#include "esp_zigbee_cluster.h"
#include "esp_zigbee_endpoint.h"
#include "zigbee_config.h"

esp_zb_ep_list_t *zigbee_create_endpoint(void)
{
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();
    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();

    //NOTE: Basic cluster is required for the device to be discoverable by the Zigbee network
    esp_zb_endpoint_config_t endpoint = {

        .endpoint = HA_ESP_VALVE_ENDPOINT,

        //NOTE: Zigbee Home Automation Profile is used for the device to be discoverable by the Zigbee network. From "zigbee_conf"
        .app_profile_id     = HA_PROFILE_ID,
        .app_device_id      = HA_DEVICE_ID,
        .app_device_version = HA_DEVICE_VERSION,
    };

    //NOTE: Basic cluster is required for the device to be discoverable by the Zigbee network
    esp_zb_attribute_list_t *basic = esp_zb_basic_cluster_create(NULL);

    esp_zb_basic_cluster_add_attr(
        basic,
        ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID,
        ESP_MANUFACTURER_NAME);

    esp_zb_basic_cluster_add_attr(
        basic,
        ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID,
        ESP_MODEL_IDENTIFIER);

    esp_zb_cluster_list_add_basic_cluster(
        cluster_list,
        basic,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    //NOTE: Identify cluster is required for the device to be discoverable by the Zigbee network
    esp_zb_cluster_list_add_identify_cluster(
        cluster_list,
        esp_zb_identify_cluster_create(NULL),
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    //NOTE: Groups cluster is required for the device to be discoverable by the Zigbee network
    esp_zb_attribute_list_t *groups = esp_zb_groups_cluster_create(NULL);

    esp_zb_cluster_list_add_groups_cluster(
        cluster_list,
        groups,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    //NOTE:
    esp_zb_attribute_list_t *scenes = esp_zb_scenes_cluster_create(NULL);

    esp_zb_cluster_list_add_scenes_cluster(
        cluster_list,
        scenes,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    //NOTE: Add the On/Off cluster to the endpoint -> Para la Valvula
    esp_zb_attribute_list_t *on_off = esp_zb_on_off_cluster_create(NULL);

    esp_zb_cluster_list_add_on_off_cluster(
        cluster_list,
        on_off,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);


   //NOTE: Add the endpoint to the endpoint list
    esp_zb_ep_list_add_ep(
        ep_list,
        cluster_list,
        endpoint);

    return ep_list;
}