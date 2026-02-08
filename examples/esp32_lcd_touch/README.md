# ESP32 LCD + touch example

Runnable on ESP32-S3 with 320×240 ST7789 (SPI) and FT6236 touch (I2C). Registers display via `lvgl_port_add_disp` and touch via `lvgl_nav_kit_add_pointer_indev`, then runs the same multi-page UI as minimal.

**Pins:** Edit `main/board_config.h` (defaults: LCD 13/47/14/21/48/12, touch 10/11).

**Run:** Copy `main/` into your project, add to `REQUIRES`: `lvgl_nav_kit lvgl log esp_lvgl_port esp_lcd driver spi_master i2c_master esp_lcd_panel_io_additions`. In `idf_component.yml` add `lvgl`, `esp_lvgl_port`, `espressif/esp_lcd_panel_io_additions`. Then `idf.py set-target esp32s3` and build.
