#pragma once
#include <stdint.h>

void Pico_Init();

void Pico_Flash_Cache_Init();
const void *Pico_Flash_Cache(const char *filename, uint32_t start_offset, uint32_t size);

void Pico_Wifi_Init(const char *ssid, const char *pass);