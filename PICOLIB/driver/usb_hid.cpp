#include "tusb.h"

static uint8_t lastKeys[6]{0, 0, 0, 0, 0};
static uint8_t lastKeyMod = 0;

static const uint8_t modMap[]
{
    HID_KEY_CONTROL_LEFT,
    HID_KEY_SHIFT_LEFT,
    HID_KEY_ALT_LEFT,
    HID_KEY_GUI_LEFT,
    HID_KEY_CONTROL_RIGHT,
    HID_KEY_SHIFT_RIGHT,
    HID_KEY_ALT_RIGHT,
    HID_KEY_GUI_RIGHT,
};

void update_key_state(uint8_t code, bool state);
void update_mouse_state(int8_t x, int8_t y, bool left, bool right);

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
    // request report if it's a keyboard/mouse
    auto protocol = tuh_hid_interface_protocol(dev_addr, instance);

    if(protocol == HID_ITF_PROTOCOL_KEYBOARD || protocol == HID_ITF_PROTOCOL_MOUSE)
        tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
    auto protocol = tuh_hid_interface_protocol(dev_addr, instance);

    if(protocol == HID_ITF_PROTOCOL_KEYBOARD)
    {
        auto keyboardReport = (hid_keyboard_report_t const*) report;

        // check for new keys down
        for(int i = 0; i < 6 && keyboardReport->keycode[i]; i++)
        {
            auto key = keyboardReport->keycode[i];
            bool found = false;
            for(int j = 0; j < 6 && lastKeys[j] && !found; j++)
                found = lastKeys[j] == key;

            if(found)
                continue;

            update_key_state(key, true);
        }

        // do the reverse and check for released keys
        for(int i = 0; i < 6 && lastKeys[i]; i++)
        {
            auto key = lastKeys[i];
            bool found = false;
            for(int j = 0; j < 6 && keyboardReport->keycode[j] && !found; j++)
                found = keyboardReport->keycode[j] == key;

            if(found)
                continue;

            update_key_state(key, false);
        }

        // ...and mods
        auto changedMods = lastKeyMod ^ keyboardReport->modifier;
        auto pressedMods = changedMods & keyboardReport->modifier;
        auto releasedMods = changedMods ^ pressedMods;
        
        for(int i = 0; i < 8; i++)
        {
            if(pressedMods & (1 << i))
                update_key_state(modMap[i], true);
            else if(releasedMods & (1 << i))
                update_key_state(modMap[i], false);
        }

        memcpy(lastKeys, keyboardReport->keycode, 6);
        lastKeyMod = keyboardReport->modifier;

        tuh_hid_receive_report(dev_addr, instance);
    }
    else if(protocol == HID_ITF_PROTOCOL_MOUSE)
    {
        auto mouseReport = (hid_mouse_report_t const*) report;

        update_mouse_state(mouseReport->x, mouseReport->y, mouseReport->buttons & MOUSE_BUTTON_LEFT, mouseReport->buttons & MOUSE_BUTTON_RIGHT);

        tuh_hid_receive_report(dev_addr, instance);
    }
}