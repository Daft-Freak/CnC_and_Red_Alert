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

enum QwSTPadIO
{
    QWSTPAD_UP_IO     = 1,
    QWSTPAD_LEFT_IO   = 2,
    QWSTPAD_RIGHT_IO  = 3,
    QWSTPAD_DOWN_IO   = 4,
    QWSTPAD_SELECT_IO = 5,
    QWSTPAD_START_IO  = 11,
    QWSTPAD_B_IO      = 12,
    QWSTPAD_Y_IO      = 13,
    QWSTPAD_A_IO      = 14,
    QWSTPAD_X_IO      = 15,
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

#ifdef FT6236_I2C
static uint8_t ft6236_data[14];
static void ft6236_int_handler(uint gpio, uint32_t events)
{
    if(gpio == FT6236_INT_PIN)
    {
        uint8_t reg = 0;
        i2c_write_blocking(FT6236_I2C, FT6236_ADDR, &reg, 1, true);
        i2c_read_blocking(FT6236_I2C, FT6236_ADDR, ft6236_data, 14, false);
    }
}
#endif

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
    gpio_set_irq_enabled_with_callback(FT6236_INT_PIN, GPIO_IRQ_LEVEL_LOW, true, ft6236_int_handler);
#endif

#ifdef QWSTPAD_I2C
    // qwst pad (tca9555)
    i2c_init(QWSTPAD_I2C, 400000);
    gpio_set_function(QWSTPAD_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(QWSTPAD_SCL_PIN, GPIO_FUNC_I2C);

    // setup for read
    uint8_t port = 0;
    i2c_write_blocking(QWSTPAD_I2C, QWSTPAD_ADDR, &port, 1, true);
#endif
}

void Pico_Input_Update()
{
    tuh_task();

#ifdef FT6236_I2C
    int num_touches = ft6236_data[FT6236_TD_STATUS] & 0xF;
    static bool touch_down = false;
    bool new_touch_down;

    for(int i = 0; i < num_touches; i++)
    {
        int x = (ft6236_data[FT6236_P1_XH + i * 6] & 0xF) << 8 | ft6236_data[FT6236_P1_XL + i * 6];
        int y = (ft6236_data[FT6236_P1_YH + i * 6] & 0xF) << 8 | ft6236_data[FT6236_P1_YL + i * 6];
        int flag = ft6236_data[FT6236_P1_XH + i * 6 + 0] >> 6;
        int id = ft6236_data[FT6236_P1_YH + i * 6 + 2] >> 4;

        if(id == 0)
        {
            mouse_x = (x * 2) / 3;
            mouse_y = (y * 2) / 3 - 60;

            if(mouse_y < 0)
                mouse_y = 0;
            else if(mouse_y >= 200)
                mouse_y = 200;

            new_touch_down = flag != FT6236_NONE && flag != FT6236_UP;

            Update_Mouse_Pos(mouse_x, mouse_y);
        }
    }

    if(new_touch_down != touch_down)
    {
        Put_Mouse_Message(VK_LBUTTON, new_touch_down);
        touch_down = new_touch_down;
    }
#endif

#ifdef QWSTPAD_I2C
    uint16_t gpio = 0;
    static uint16_t last_gpio = 0xFFFF;
    static absolute_time_t last_gpio_time = 0;

    i2c_read_blocking(QWSTPAD_I2C, QWSTPAD_ADDR, (uint8_t *)&gpio, 2, false);
    gpio = ~gpio;

    // get time delta
    auto gpio_time = get_absolute_time();
    auto gpio_dt = absolute_time_diff_us(last_gpio_time, gpio_time) / 10000; // 100ths of a second
    last_gpio_time = delayed_by_ms(last_gpio_time, gpio_dt * 10);

    auto changed = gpio ^ last_gpio;
    last_gpio = gpio;
    
    // directional buttons -> mouse
    // TODO: accelerate?
    if(gpio & (1 << QWSTPAD_UP_IO))
        mouse_y -= gpio_dt;
    else if(gpio & (1 << QWSTPAD_DOWN_IO))
        mouse_y += gpio_dt;

    if(gpio & (1 << QWSTPAD_LEFT_IO))
        mouse_x -= gpio_dt;
    else if(gpio & (1 << QWSTPAD_RIGHT_IO))
        mouse_x += gpio_dt;

    const auto any_dir_mask =(1 << QWSTPAD_UP_IO | 1 << QWSTPAD_LEFT_IO | 1 << QWSTPAD_RIGHT_IO | 1 << QWSTPAD_DOWN_IO);

    if(gpio & any_dir_mask)
    {
        // clamp
        if(mouse_x < 0)
            mouse_x = 0;
        else if(mouse_x >= 320)
            mouse_x = 319;

        if(mouse_y < 0)
            mouse_y = 0;
        else if(mouse_y >= 200)
            mouse_y = 199;
        
        Update_Mouse_Pos(mouse_x, mouse_y);
    }
    else if(changed & any_dir_mask)
    {
        // move the mouse slightly so we stop scrolling
        if(mouse_x == 0)
            mouse_x = 1;
        else if(mouse_x == 319)
            mouse_x--;

        if(mouse_y == 0)
            mouse_y = 1;
        else if(mouse_y == 199)
            mouse_y--;

        Update_Mouse_Pos(mouse_x, mouse_y);
    }

    // A/B -> mouse buttons
    if(changed & (1 << QWSTPAD_A_IO))
        Put_Mouse_Message(VK_LBUTTON, gpio & (1 << QWSTPAD_A_IO));
    if(changed & (1 << QWSTPAD_B_IO))
        Put_Mouse_Message(VK_RBUTTON, gpio & (1 << QWSTPAD_B_IO));

    // start -> esc
    if(changed & (1 << QWSTPAD_START_IO))
        TheKeyboard->Put_Key_Message(VK_ESCAPE, gpio & (1 << QWSTPAD_START_IO));
#endif
}