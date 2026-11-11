idf_component_register(
    SRCS
        "main.cpp"
        "clock_machine.cpp"
        "clock_machine_states.cpp"
        "display.cpp"
        "wifi_time.cpp"
        "rotary_encoder.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        esp_http_server
        nvs_flash
        esp_wifi
        esp_netif
        esp_event
        esp_timer
        driver
        esp_adc
        LovyanGFX
)

