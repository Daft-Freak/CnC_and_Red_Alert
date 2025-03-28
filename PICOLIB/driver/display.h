#pragma once
#include <cstdint>

void init_display();
void init_display_core1();

void update_display(uint32_t time);
void set_screen_palette(const uint8_t *colours, int num_cols);

bool display_render_needed();

uint8_t *get_framebuffer();