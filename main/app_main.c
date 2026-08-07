#include "nvs_flash.h"

#include "zigbee_stack.h"


void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    zigbee_start();
}