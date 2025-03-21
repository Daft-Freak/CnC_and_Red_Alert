#include <stdio.h>

#include "pico/stdlib.h"

#include "display.h"
#include "psram.h"
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

void Pico_Init()
{
    stdio_init_all();

    size_t psramSize = psram_init(PSRAM_CS_PIN);
    printf("detected %i bytes PSRAM\n", psramSize);
    *(uint32_t *)PSRAM_LOCATION = psramSize;

    f_mount(&fs, "", 0);
    f_chdir("/CnC/");

    Pico_Flash_Cache_Init();

    init_display();
}