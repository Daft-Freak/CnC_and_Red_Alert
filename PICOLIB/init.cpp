#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#ifdef WIFI_ENABLED
#include "pico/cyw43_arch.h"
#endif
#include "tusb.h"

#include "driver/audio.h"
#include "display.h"
#include "psram.h"
#include "mem.h"
#include "fatfs/ff.h"

#include "keyboard.h"
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
static uint8_t mouse_buttons = 0;
extern WWKeyboardClass *TheKeyboard;

// input glue
void update_key_state(uint8_t code, bool state)
{
    if(TheKeyboard)
        TheKeyboard->Put_Key_Message(code, !state);
}

void update_mouse_state(int8_t x, int8_t y, bool left, bool right)
{
    int tmp_x = mouse_x + x;
    int tmp_y = mouse_y + y;
    mouse_x = tmp_x < 0 ? 0 : (tmp_x > 319 ? 319 : tmp_x);
    mouse_y = tmp_y < 0 ? 0 : (tmp_y > 199 ? 199 : tmp_y);

    Update_Mouse_Pos(mouse_x, mouse_y);

    // also need to forward the buttons to the keyboard
    if(TheKeyboard)
    {
        bool old_left = mouse_buttons & 1;
        bool old_right = mouse_buttons & 2;

        if(old_left != left)
        {
            TheKeyboard->Put_Key_Message(VK_LBUTTON, !left);
            TheKeyboard->Put(mouse_x);
            TheKeyboard->Put(mouse_y);
        }
        if(old_right != right)
        {
            TheKeyboard->Put_Key_Message(VK_RBUTTON, !right);
            TheKeyboard->Put(mouse_x);
            TheKeyboard->Put(mouse_y);
        }

        mouse_buttons = (left ? 1 : 0) | (right ? 2 : 0);
    }
}

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

    tusb_init();

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