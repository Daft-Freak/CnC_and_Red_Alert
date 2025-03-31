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
#define LCD_MAX_CLOCK 62500000

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

#elif EXTRA_BOARD_VGABOARD

#define AUDIO_I2S_PIO 0
#define AUDIO_I2S_CLOCK_PIN_BASE 27
#define AUDIO_I2S_DATA_PIN       26

#define SD_SCK   5
#define SD_MOSI 18
#define SD_MISO 19
#define SD_CS   22

#endif