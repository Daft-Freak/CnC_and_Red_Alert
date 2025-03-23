#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "memflag.h"

#include "psram.h"

void (*Memory_Error)(void) = NULL;
void (*Memory_Error_Exit)(char *string) = NULL;

void *PSRAM_Alloc(size_t size)
{
    // very dumb allocator
    printf("psalloc %u\n", size);

    auto ptr = (uint8_t *)PSRAM_LOCATION;
    auto end = ptr + 8 * 1024 * 1024; // whoops, hardcoded size

    while(ptr < end)
    {
        uint32_t head = *(uint32_t *)ptr;

        // allocated block
        if(head & (1 << 31))
        {
            ptr += (head & 0xFFFFFF);
            continue;
        }

        printf("\tfree %u at %x\n", head, ptr - (uint8_t *)PSRAM_LOCATION);

        size += 4; // header
        size = (size + 3) & ~3; // align

        *(uint32_t *)ptr = (1 << 31) | size;

        *(uint32_t *)(ptr + size) = head - size;

        return ptr + 4;
    }

    return NULL;
}

void Force_VM_Page_In(void *buffer, int length)
{
}

void *Alloc(unsigned long bytes_to_alloc, MemoryFlagType flags)
{
	void *ptr;
    
    if(flags & MEM_FIXED_HEAP)
        ptr = PSRAM_Alloc(bytes_to_alloc);
    else
        ptr = new char[bytes_to_alloc];

	if(!ptr && Memory_Error)
		Memory_Error();

	if(ptr && (flags & MEM_CLEAR))
		memset(ptr, 0, bytes_to_alloc);

	return ptr;
}

void Free(void const *pointer)
{
    if(pointer)
        delete[] (char *)pointer;
}

void Mem_Copy(void const *source, void *dest, unsigned long bytes_to_copy)
{
    memcpy(dest, source, bytes_to_copy);
}

void *Resize_Alloc(void *original_ptr, unsigned long new_size_in_bytes)
{
    // it's a good thing that this isn't getting used, because it's wrong!
    void *ptr = realloc(original_ptr, new_size_in_bytes);

    if(!ptr && Memory_Error)
		Memory_Error();

    return ptr;
}

long Ram_Free(MemoryFlagType flag)
{
    // used by WSA to decide allocation size
    
    extern char __StackLimit; 
    extern char end;

    // to match _sbrk
    uint32_t heap_size = &__StackLimit - &end;

    return heap_size - mallinfo().uordblks;
}