#pragma once
#include <cstdint>

void audio_callback(int16_t *buffer, int count);

void init_audio();
//void update_audio();

bool audio_available();