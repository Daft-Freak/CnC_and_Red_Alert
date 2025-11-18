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

#elif EXTRA_BOARD_SD28
#define AUDIO_I2S_PIO 0
#define AUDIO_I2S_MUTE_PIN 25
#define AUDIO_I2S_DATA_PIN 26
#define AUDIO_I2S_CLOCK_PIN_BASE 27

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

#define DBI_8BIT
#define LCD_CS_PIN 2
#define LCD_DC_PIN 3
#define LCD_SCK_PIN 6 // WR
#define LCD_RD_PIN 7
#define LCD_MOSI_PIN 9 // D0
#define LCD_RESET_PIN 21
#define LCD_BACKLIGHT_PIN 23
#define LCD_VSYNC_PIN 22

#define LCD_MAX_CLOCK 15000000
#define LCD_ROTATION 270

#define SD_SCK  18
#define SD_MOSI 19
#define SD_MISO 20
#define SD_CS   17

#define SD28PAD_I2C i2c_default
#define SD28PAD_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN
#define SD28PAD_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN

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

// so far the only self-contained board
#elif defined(ADAFRUIT_FRUIT_JAM)
//#define AUDIO_I2S_CLOCK_PIN_BASE ADAFRUIT_FRUIT_JAM_I2S_BCLK_PIN
//#define AUDIO_I2S_DATA_PIN       ADAFRUIT_FRUIT_JAM_I2S_DIN_PIN

// 720x400
#define DISPLAY_WIDTH  360
#define DISPLAY_HEIGHT 200

#define DVI_CLK_P ADAFRUIT_FRUIT_JAM_DVI_CKP_PIN
#define DVI_D0_P  ADAFRUIT_FRUIT_JAM_DVI_D0P_PIN
#define DVI_D1_P  ADAFRUIT_FRUIT_JAM_DVI_D1P_PIN
#define DVI_D2_P  ADAFRUIT_FRUIT_JAM_DVI_D2P_PIN

#define SD_SCK    ADAFRUIT_FRUIT_JAM_SD_SCK_PIN
#define SD_MOSI   ADAFRUIT_FRUIT_JAM_SD_MOSI_PIN
#define SD_MISO   ADAFRUIT_FRUIT_JAM_SD_MISO_PIN
#define SD_CS     ADAFRUIT_FRUIT_JAM_SD_CS_PIN

// like the VGA board above, this is a "make sure we pull up"
#define SD_DAT1   ADAFRUIT_FRUIT_JAM_SDIO_DATA1_PIN
#define SD_DAT2   ADAFRUIT_FRUIT_JAM_SDIO_DATA2_PIN

#define PIO_USB_DP_PIN_DEFAULT            ADAFRUIT_FRUIT_JAM_USB_HOST_DATA_PLUS_PIN
#define PICO_DEFAULT_PIO_USB_VBUSEN_PIN   ADAFRUIT_FRUIT_JAM_USB_HOST_5V_POWER_PIN
#define PICO_DEFAULT_PIO_USB_VBUSEN_STATE 1
#endif
