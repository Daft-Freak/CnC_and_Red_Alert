#pragma once
#include <cstdint>

void init_display();

void update_display(uint32_t time);
void set_screen_palette(const uint8_t *colours, int num_cols);

bool display_render_needed();

uint8_t *get_framebuffer();

// not really a "lock" as there isn't an "unlock"
// but more of a "wait until it isn't in use"
void display_lock_framebuffer();

void display_set_cursor(uint8_t *data, int w, int h);
void display_set_cursor_pos(int x, int y);