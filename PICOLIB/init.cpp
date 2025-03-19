#include "pico/stdlib.h"

#include "storage.h"
#include "fatfs/ff.h"

static FATFS fs;

void Pico_Init()
{
    stdio_init_all();

    storage_init();
    f_mount(&fs, "", 0);
    f_chdir("/CnC/");
}