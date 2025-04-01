#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#ifdef WIFI_ENABLED
#include "pico/cyw43_arch.h"
#endif

#include "driver/audio.h"
#include "display.h"
#include "psram.h"
#include "mem.h"
#include "fatfs/ff.h"

#include "picolib.h"

static FATFS fs;

#ifdef PIMORONI_PICO_PLUS2_RP2350
#define PSRAM_CS_PIN PIMORONI_PICO_PLUS2_PSRAM_CS_PIN
#elif defined(PIMORONI_PICO_PLUS2_W_RP2350)
#define PSRAM_CS_PIN PIMORONI_PICO_PLUS2_W_PSRAM_CS_PIN
#else
#error "No PSRAM CS!"
#endif

static void __not_in_flash_func(Core1_Main)()
{
    init_display_core1();
    while(true) __wfe();
}

void Pico_Init()
{
    stdio_init_all();

    size_t psramSize = psram_init(PSRAM_CS_PIN);
    printf("detected %i bytes PSRAM\n", psramSize);
    PSRAM_Alloc_Init();

    f_mount(&fs, "", 0);
    f_chdir("/CnC/");

    Pico_Flash_Cache_Init();

    Pico_Input_Init();

    init_display();
    init_audio();

    multicore_launch_core1(Core1_Main);
}

void Pico_Wifi_Init(const char *ssid, const char *pass)
{
#ifdef WIFI_ENABLED
    if(cyw43_arch_init() != 0)
    {
        printf("cyw43_arch_init failed!\n");
        return;
    }

    cyw43_arch_enable_sta_mode();

    if(cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("failed to connect\n");
        return;
    }

    printf("wifi connected.\n");
#endif
}