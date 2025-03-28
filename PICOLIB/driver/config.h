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

#elif EXTRA_BOARD_VGABOARD

#define SD_SCK   5
#define SD_MOSI 18
#define SD_MISO 19
#define SD_CS   22

#endif