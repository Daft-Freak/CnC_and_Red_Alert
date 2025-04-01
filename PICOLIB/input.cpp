#include "tusb.h"

#include "picolib.h"
#include "keyboard.h"
#include "mouse.h"

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

void Pico_Input_Init()
{
    tusb_init();
}

void Pico_Input_Update()
{
    tuh_task();
}