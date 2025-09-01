#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "esp_heap_caps.h"

#include "memflag.h"

void (*Memory_Error)(void) = NULL;
void (*Memory_Error_Exit)(char *string) = NULL;

void Force_VM_Page_In(void *buffer, int length)
{
}

void *Alloc(unsigned long bytes_to_alloc, MemoryFlagType flags)
{
	void *ptr;
    
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
    return heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
}