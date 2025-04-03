#include "tusb.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "picolib.h"
#include "keyboard.h"
#include "mouse.h"

#include "config.h"

enum FT6236Reg
{
    FT6236_DEV_MODE = 0,
    FT6236_GEST_ID,
    FT6236_TD_STATUS,

    FT6236_P1_XH,
    FT6236_P1_XL,
    FT6236_P1_YH,
    FT6236_P1_YL,
    FT6236_P1_WEIGHT,
    FT6236_P1_MISC,

    FT6236_P2_XH,
    FT6236_P2_XL,
    FT6236_P2_YH,
    FT6236_P2_YL,
    FT6236_P2_WEIGHT,
    FT6236_P2_MISC,
};

enum FT6236Event
{
    FT6236_DOWN = 0,
    FT6236_UP,
    FT6236_CONTACT,
    FT6236_NONE
};

static int16_t mouse_x = 0, mouse_y = 0;
static uint8_t mouse_buttons = 0;
extern WWKeyboardClass *TheKeyboard;

void Put_Mouse_Message(int key, bool down)
{
    if(!TheKeyboard)
        return;

    TheKeyboard->Put_Key_Message(key, !down);
    TheKeyboard->Put(mouse_x);
    TheKeyboard->Put(mouse_y);
}

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
            Put_Mouse_Message(VK_LBUTTON, left);
        if(old_right != right)
            Put_Mouse_Message(VK_RBUTTON, right);

        mouse_buttons = (left ? 1 : 0) | (right ? 2 : 0);
    }
}

void Pico_Input_Init()
{
    tusb_init();

#ifdef FT6236_I2C
    // presto touch (FT6236)
    i2c_init(FT6236_I2C, 400000);
    gpio_set_function(FT6236_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(FT6236_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(FT6236_SDA_PIN);
    gpio_pull_up(FT6236_SCL_PIN);

    // int
    gpio_set_dir(FT6236_INT_PIN, false);
    gpio_set_function(FT6236_INT_PIN, GPIO_FUNC_SIO);
#endif
}

void Pico_Input_Update()
{
    tuh_task();

#ifdef FT6236_I2C
    if(!gpio_get(FT6236_INT_PIN))
    {
        uint8_t reg = 0;
        uint8_t data[16];
        i2c_write_blocking(FT6236_I2C, FT6236_ADDR, &reg, 1, true);
        i2c_read_blocking(FT6236_I2C, FT6236_ADDR, data, 16, false);

        int num_touches = data[FT6236_TD_STATUS] & 0xF;
        static bool touch_down = false;

        if(num_touches)
        {
            for(int i = 0; i < num_touches; i++)
            {
                int x = (data[FT6236_P1_XH + i * 6] & 0xF) << 8 | data[FT6236_P1_XL + i * 6];
                int y = (data[FT6236_P1_YH + i * 6] & 0xF) << 8 | data[FT6236_P1_YL + i * 6];
                int flag = data[FT6236_P1_XH + i * 6 + 0] >> 6;
                int id = data[FT6236_P1_YH + i * 6 + 2] >> 4;

                if(id == 0)
                {
                    mouse_x = (x * 2) / 3;
                    mouse_y = (y * 2) / 3 - 60;

                    if(mouse_y < 0)
                        mouse_y = 0;
                    else if(mouse_y >= 200)
                        mouse_y = 200;

                    bool new_touch_down = flag != FT6236_NONE && flag != FT6236_CONTACT;

                    Update_Mouse_Pos(mouse_x, mouse_y);

                    if(new_touch_down != touch_down)
                    {
                        Put_Mouse_Message(VK_LBUTTON, new_touch_down);
                        touch_down = new_touch_down;
                    }
                }
            }
        }
    }
#endif
}