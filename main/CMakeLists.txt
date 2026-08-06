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
        esp_driver_uart
        ieee802154
)
