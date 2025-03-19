#include "pico/stdlib.h"

#include "storage.h"

void Pico_Init()
{
    stdio_init_all();

    storage_init();
}