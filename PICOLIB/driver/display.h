#pragma once
#include <cstdint>

void pre_init_display();
void init_display();
void init_display_core1();

void update_display(uint32_t time);
void set_screen_palette(const uint8_t *colours, int num_cols);

bool display_render_needed();

uint8_t *get_framebuffer();

void display_set_cursor(uint8_t *data, int w, int h);
void display_set_cursor_pos(int x, int y);