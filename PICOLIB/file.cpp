#include <stdio.h>

#include "file.h"

void *IO_Open_File(const char *filename, int mode)
{
    printf("open %s\n", filename);
    return NULL;
}

void IO_Close_File(void *handle)
{
}

bool IO_Read_File(void *handle, void *buffer, size_t count, size_t &actual_read)
{
    return false;
}

bool IO_Write_File(void *handle, const void *buffer, size_t count, size_t &actual_written)
{
    return false;
}

size_t IO_Seek_File(void *handle, size_t offset, int origin)
{
    return 0;
}

size_t IO_Get_File_Size(void *handle)
{
    return 0;
}

bool IO_Delete_File(const char *filename)
{
    return false;
}

bool Find_First_File(const char *path_glob, FindFileState &state)
{
    printf("find first %s\n", path_glob);
    return false;
}

bool Find_Next_File(FindFileState &state)
{
    return false;
}

void End_Find_File(FindFileState &state)
{
    if(state.data)
    {
        state.data = NULL;
    }
}

uint64_t Disk_Space_Available()
{
    return 1024 * 1024 * 1024;
}
