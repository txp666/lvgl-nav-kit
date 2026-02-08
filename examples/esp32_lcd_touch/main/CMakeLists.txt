# ESP32-S3 LCD + touch example. Requires lvgl_nav_kit esp_lvgl_port esp_lcd esp_lcd_panel_io_additions
idf_component_register(
    SRCS "main.cc"
    INCLUDE_DIRS "."
    REQUIRES
        lvgl_nav_kit
        lvgl
        log
        esp_lvgl_port
        esp_lcd
        driver
        spi_master
        i2c_master
        esp_lcd_panel_io_additions
)
