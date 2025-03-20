#pragma once

#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSRAM_LOCATION _u(0x11000000)

size_t psram_init(uint cs_pin);

#ifdef __cplusplus
}
#endif