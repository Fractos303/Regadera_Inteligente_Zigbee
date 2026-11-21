idf_component_register(
    SRC_DIRS
        "."
        "../app"
        "../drivers"
        "../zigbee"

    INCLUDE_DIRS
        "."
        "../app"
        "../drivers"
        "../zigbee"
        "../config"

    PRIV_REQUIRES
        nvs_flash
        esp_adc
        esp_driver_gpio
        esp_driver_uart
        esp_driver_i2c
        ieee802154
        esp_driver_rmt
)
