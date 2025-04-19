#pragma once

#if EXTRA_BOARD_DISPLAY_PACK

#define SD_SCK  10
#define SD_MOSI 11
#define SD_MISO 8
#define SD_CS   9

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

#define LCD_DC_PIN        16
#define LCD_CS_PIN        17
#define LCD_SCK_PIN       18
#define LCD_MOSI_PIN      19
#define LCD_BACKLIGHT_PIN 20
#define LCD_VSYNC_PIN     21

#define LCD_ROTATION 90
#define LCD_MAX_CLOCK 75000000 // should be 62500000, but can't get that from 150MHz so overclock

#elif EXTRA_BOARD_STAMP_CARRIER
#define DISPLAY_WIDTH  360
#define DISPLAY_HEIGHT 200

#define DVI_CLK_P 14
#define DVI_D0_P  12
#define DVI_D1_P  18
#define DVI_D2_P  16

// these are not the slot on the carrier, it conflicts with the PSRAM CS
#define SD_SCK    39
#define SD_MOSI   37
#define SD_MISO   38
#define SD_CS     36

#elif EXTRA_BOARD_PRESTO

#define DPI_DATA_PIN_BASE  1
#define DPI_SYNC_PIN_BASE 19
#define DPI_CLOCK_PIN     22

#define DPI_MODE_CLOCK 12500000 // very ish

#define DPI_MODE_H_FRONT_PORCH     4
#define DPI_MODE_H_SYNC_WIDTH     16
#define DPI_MODE_H_BACK_PORCH     30
#define DPI_MODE_H_ACTIVE_PIXELS 480

#define DPI_MODE_V_FRONT_PORCH     5
#define DPI_MODE_V_SYNC_WIDTH      8
#define DPI_MODE_V_BACK_PORCH      5
#define DPI_MODE_V_ACTIVE_LINES  480

#define DPI_SPI_INIT spi1
#define DPI_ST7701

#define DPI_BIT_REVERSE

#define DPI_SCALE_1_5 // remove if you like borders

#define FT6236_I2C i2c1
#define FT6236_SDA_PIN 30
#define FT6236_SCL_PIN 31
#define FT6236_INT_PIN 32
#define FT6236_ADDR 0x48

#define LCD_CS_PIN        28
#define LCD_DC_PIN        -1
#define LCD_SCK_PIN       26
#define LCD_MOSI_PIN      27
#define LCD_BACKLIGHT_PIN 45
#define LCD_RESET_PIN     44

#define SD_SCK  34
#define SD_MOSI 35
#define SD_MISO 36
#define SD_CS   39

#define QWSTPAD_I2C i2c0
#define QWSTPAD_SDA_PIN 40
#define QWSTPAD_SCL_PIN 41
#define QWSTPAD_ADDR 0x21

#elif EXTRA_BOARD_VGABOARD

#define AUDIO_I2S_PIO 0
#define AUDIO_I2S_CLOCK_PIN_BASE 27
#define AUDIO_I2S_DATA_PIN       26

#define SD_SCK   5
#define SD_MOSI 18
#define SD_MISO 19
#define SD_CS   22

#endif
