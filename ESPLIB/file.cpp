#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "esp_vfs_fat.h"

#include "file.h"

// doesn't seem to be a working "chdir" anywhere
char path_prefix[10]{0};

void *IO_Open_File(const char *filename, int mode)
{
    int p_mode;
    if(mode == READ)
        p_mode = O_RDONLY;
    else if(mode == WRITE)
        p_mode = O_WRONLY | O_CREAT | O_TRUNC;
    else if(mode == (READ | WRITE))
        p_mode = O_RDWR | O_CREAT | O_TRUNC;
    else
        return nullptr;

    char prefixed_path[300];
    snprintf(prefixed_path, sizeof(prefixed_path), "%s%s", path_prefix, filename);

    int fd = open(prefixed_path, p_mode, 0);

    if(fd < 0)
        return nullptr;

    return (void *)fd;
}

void IO_Close_File(void *handle)
{
    auto fd = (int)handle;
    close(fd);
}

bool IO_Read_File(void *handle, void *buffer, size_t count, size_t &actual_read)
{
    auto fd = (int)handle;
    auto res = read(fd, buffer, count);

    if(res < 0)
    {
        actual_read = 0;
        return false;
    }

    actual_read = res;
    return true;
}

bool IO_Write_File(void *handle, const void *buffer, size_t count, size_t &actual_written)
{
    auto fd = (int)handle;
    auto res = write(fd, buffer, count);

    if(res < 0)
    {
        actual_written = 0;
        return false;
    }

    actual_written = res;
    return true;
}

size_t IO_Seek_File(void *handle, size_t offset, int origin)
{
    auto fd = (int)handle;
    return lseek(fd, offset, origin);
}

size_t IO_Get_File_Size(void *handle)
{
    auto fd = (int)handle;
    off_t length = lseek(fd, 0, SEEK_END);

    lseek(fd, 0, SEEK_SET);

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
