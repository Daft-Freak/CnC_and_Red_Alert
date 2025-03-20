#include "pico/stdlib.h"

#include "storage.h"
#include "fatfs/ff.h"

#include "picolib.h"

static FATFS fs;

void Pico_Init()
{
    stdio_init_all();

    f_mount(&fs, "", 0);
    f_chdir("/CnC/");

    Pico_Flash_Cache_Init();
}