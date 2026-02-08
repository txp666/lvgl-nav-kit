/**
 * @file board_config.h
 * Board config: LCD and touch pins, resolution. Default: ESP32-S3 + 320x240 ST7789 (SPI) + FT6236 (I2C).
 */
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <driver/gpio.h>

/* Display (ST7789 SPI) */
#define LCD_SPI_HOST            SPI3_HOST
#define LCD_MOSI_PIN            GPIO_NUM_13
#define LCD_CLK_PIN             GPIO_NUM_47
#define LCD_CS_PIN              GPIO_NUM_14
#define LCD_DC_PIN              GPIO_NUM_21
#define LCD_RST_PIN             GPIO_NUM_48
#define LCD_BACKLIGHT_PIN       GPIO_NUM_12

#define LCD_WIDTH               320
#define LCD_HEIGHT              240
#define LCD_SPI_MODE             2
#define LCD_SWAP_XY             1
#define LCD_MIRROR_X            1
#define LCD_MIRROR_Y            0
#define LCD_INVERT_COLORS       1
#define LCD_OFFSET_X             0
#define LCD_OFFSET_Y             0

/* Touch (FT6236/FT6x36 I2C) */
#define TOUCH_I2C_NUM           I2C_NUM_1
#define TOUCH_SDA_PIN           GPIO_NUM_10
#define TOUCH_SCL_PIN           GPIO_NUM_11
#define TOUCH_I2C_ADDR           0x38
#define TOUCH_I2C_HZ             400000

#endif /* BOARD_CONFIG_H */
