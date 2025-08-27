#include "display.h"
#include "config.h"

static uint8_t framebuffer[320 * 200];

void init_display()
{

}

void update_display(uint32_t time)
{

}
void set_screen_palette(const uint8_t *colours, int num_cols)
{

}

bool display_render_needed()
{
    return false;
}

uint8_t *get_framebuffer()
{
    return framebuffer;
}

void display_set_cursor(uint8_t *data, int w, int h)
{

}

void display_set_cursor_pos(int x, int y)
{

}