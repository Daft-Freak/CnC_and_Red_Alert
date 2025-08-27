#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "esp_vfs_fat.h"

#include "file.h"

// doesn't seem to be a working "chdir" anywhere
char path_prefix[10]{0};

void *IO_Open_File(const char *filename, int mode)
{
    const char *mode_str;

    if(mode == READ)
        mode_str = "rb";
    else if(mode == WRITE)
        mode_str = "wb";
    else if(mode == (READ | WRITE))
        mode_str = "w+b";
    else
        return NULL;

    
    char prefixed_path[300];
    snprintf(prefixed_path, sizeof(prefixed_path), "%s%s", path_prefix, filename);

    return fopen(prefixed_path, mode_str);
}

void IO_Close_File(void *handle)
{
    auto file = (FILE *)handle;
    fclose(file);
}

bool IO_Read_File(void *handle, void *buffer, size_t count, size_t &actual_read)
{
    auto file = (FILE *)handle;
    actual_read = fread(buffer, 1, count, file);
    return ferror(file) == 0;
}

bool IO_Write_File(void *handle, const void *buffer, size_t count, size_t &actual_written)
{
    auto file = (FILE *)handle;
    actual_written = fwrite(buffer, 1, count, file);
    return ferror(file) == 0;
}

size_t IO_Seek_File(void *handle, size_t offset, int origin)
{
    auto file = (FILE *)handle;
    fseek(file, offset, origin);
    return ftell(file);
}

size_t IO_Get_File_Size(void *handle)
{
    auto file = (FILE *)handle;
    long pos = ftell(file);

    fseek(file, 0, SEEK_END);

    long length = ftell(file);

    fseek(file, pos, SEEK_SET);

    return length;
}

bool IO_Delete_File(const char *filename)
{
    return unlink(filename) == 0;
}


bool Find_First_File(const char *path_glob, FindFileState &state)
{
    return false;
}

bool Find_Next_File(FindFileState &state)
{
    return false;
}

void End_Find_File(FindFileState &state)
{

}

uint64_t Disk_Space_Available()
{
    // TODO: is there a generic way of doing this?

    DWORD free_clusters = 0;
    FATFS *fs;
    if(f_getfree("", &free_clusters, &fs) != FR_OK)
        return 0;

    return uint64_t(free_clusters) * fs->csize * 512;
}
