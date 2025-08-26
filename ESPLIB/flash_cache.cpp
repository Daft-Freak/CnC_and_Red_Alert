#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"

void Pico_Flash_Cache_Init()
{
}

const void *Pico_Flash_Cache(const char *filename, uint32_t start_offset, uint32_t size)
{
    printf("cache %s off %u size %u\n", filename, start_offset, size);

    return nullptr;
}