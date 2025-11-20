#pragma once
#include <cstdint>

void nina_spi_init();

bool nina_connect_timeout(const char *ssid, const char *pass, uint32_t timeout_ms);