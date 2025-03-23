#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico.h"
#include "pico/binary_info.h"

#include "hardware/flash.h"
#include "hardware/sync.h"

#include "file.h"

#define FLASH_CACHE_SIZE (10 * 1024 * 1024)
#define FLASH_CACHE_START (PICO_FLASH_SIZE_BYTES - FLASH_CACHE_SIZE)
static_assert(FLASH_CACHE_START > 0);

void Pico_Flash_Cache_Init()
{
    bi_decl(bi_block_device(
        BINARY_INFO_MAKE_TAG('D', 'F'),
        "MIX Cache",
        XIP_BASE + FLASH_CACHE_START,
        FLASH_CACHE_SIZE,
        nullptr,
        BINARY_INFO_BLOCK_DEV_FLAG_READ | BINARY_INFO_BLOCK_DEV_FLAG_WRITE | BINARY_INFO_BLOCK_DEV_FLAG_PT_NONE
    ))

    const uint8_t *ptr = (const uint8_t *)XIP_NOCACHE_NOALLOC_BASE + FLASH_CACHE_START;

    if(memcmp(ptr, "MIXCACHE", 8) == 0)
    {
        printf("MIX cache found\n");
        return;
    }

    printf("Initialising MIX cache...\n");

    const int step = FLASH_SECTOR_SIZE * 16;
    for(uint32_t off = 0; off < FLASH_CACHE_SIZE; off += step)
    {
        auto status = save_and_disable_interrupts();
        flash_range_erase(FLASH_CACHE_START + off, step);
        restore_interrupts(status);

        printf("%u/%u\r", off, FLASH_CACHE_SIZE);
    }

    // write the header
    uint8_t buf[FLASH_PAGE_SIZE];
    memset(buf, 0xFF, FLASH_PAGE_SIZE);
    memcpy(buf, "MIXCACHE", 8);

    auto status = save_and_disable_interrupts();
    flash_range_program(FLASH_CACHE_START, buf, FLASH_PAGE_SIZE);
    restore_interrupts(status);

    printf("\nDone!\n");
}

const void *Pico_Flash_Cache(const char *filename, uint32_t start_offset, uint32_t size)
{
    printf("cache %s off %u size %u\n", filename, start_offset, size);

    // look for existing
    uint32_t scan_offset = FLASH_CACHE_START + FLASH_PAGE_SIZE;
    while(scan_offset < FLASH_CACHE_START + FLASH_CACHE_SIZE)
    {
        const uint8_t *ptr = (const uint8_t *)XIP_NOCACHE_NOALLOC_BASE + scan_offset;

        if(*ptr == 0xFF)
            break;

        uint32_t size = *(uint32_t *)(ptr + 12) >> 8; // the first byte is the end of the name

        printf("found %s size %u at %x\n", ptr, size, scan_offset);

        // found it
        if(strcmp(filename, (const char *)ptr) == 0)
            return ptr + 16;

        scan_offset += size + 16;
        // re-align
        scan_offset = (scan_offset + (FLASH_PAGE_SIZE - 1)) & ~(FLASH_PAGE_SIZE - 1);
    }

    // didn't find it
    auto file = Open_File(filename, READ);
    if(file == -1)
    {
        printf("failed to open file!\n");
        return NULL;
    }

    printf("storing %s at %x...\n", filename, scan_offset);

    Seek_File(file, start_offset, SEEK_SET);

    // header in first page
    // assuming the file is larger than one page, which it should be
    uint8_t buf[FLASH_PAGE_SIZE]{};
    strcpy((char *)buf, filename);
    *(uint32_t *)(buf + 12) = size << 8;
    
    size_t read = 0;
    read = Read_File(file, buf + 16, FLASH_PAGE_SIZE - 16);

    auto status = save_and_disable_interrupts();
    flash_range_program(scan_offset, buf, FLASH_PAGE_SIZE);
    restore_interrupts(status);

    // flash the rest
    uint32_t remaining = size - read;
    uint32_t flash_offset = scan_offset + FLASH_PAGE_SIZE;

    while(remaining)
    {
        read = 0;
        read = Read_File(file, buf, FLASH_PAGE_SIZE);

        auto status = save_and_disable_interrupts();
        flash_range_program(flash_offset, buf, FLASH_PAGE_SIZE);
        restore_interrupts(status);

        flash_offset += FLASH_PAGE_SIZE;

        if(read > remaining)
            remaining = 0;
        else
            remaining -= read;

        printf("%u/%u\r", size - remaining, size);
    }

    printf("\n");

    Close_File(file);

    return (const void *)(XIP_NOCACHE_NOALLOC_BASE + scan_offset + 16);
}