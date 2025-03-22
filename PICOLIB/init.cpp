#include <stdio.h>

#include "pico/stdlib.h"
#include "tusb.h"

#include "display.h"
#include "psram.h"
#include "fatfs/ff.h"

#include "mouse.h"
#include "picolib.h"

static FATFS fs;

#ifdef PIMORONI_PICO_PLUS2_RP2350
#define PSRAM_CS_PIN PIMORONI_PICO_PLUS2_PSRAM_CS_PIN
#elif defined(PIMORONI_PICO_PLUS2_W_RP2350)
#define PSRAM_CS_PIN PIMORONI_PICO_PLUS2_W_PSRAM_CS_PIN
#else
#error "No PSRAM CS!"
#endif

static uint16_t mouse_x = 0, mouse_y = 0;

// input glue
void update_key_state(uint8_t code, bool state)
{
    
}

void update_mouse_state(int8_t x, int8_t y, bool left, bool right)
{
    int tmp_x = mouse_x + x;
    int tmp_y = mouse_y + y;
    mouse_x = tmp_x < 0 ? 0 : (tmp_x > 319 ? 319 : tmp_x);
    mouse_y = tmp_y < 0 ? 0 : (tmp_y > 199 ? 199 : tmp_y);

    Update_Mouse_Pos(mouse_x, mouse_y);
    // also need to forward the event to the keyboard
}

void Pico_Init()
{
    stdio_init_all();

    size_t psramSize = psram_init(PSRAM_CS_PIN);
    printf("detected %i bytes PSRAM\n", psramSize);
    *(uint32_t *)PSRAM_LOCATION = psramSize;

    f_mount(&fs, "", 0);
    f_chdir("/CnC/");

    Pico_Flash_Cache_Init();

    tusb_init();

    init_display();
}