#include <stdio.h>
#include <string.h>

#include "file.h"

#include "fatfs/ff.h"

void *IO_Open_File(const char *filename, int mode)
{
    int ff_mode;

    if(mode == READ)
        ff_mode = FA_READ;
    else if(mode == WRITE)
        ff_mode = FA_CREATE_ALWAYS | FA_WRITE;
    else if(mode == (READ | WRITE))
        ff_mode = FA_CREATE_ALWAYS | FA_READ | FA_WRITE;
    else
        return NULL;

    auto fp = new FIL;

    if(f_open(fp, filename, ff_mode) != FR_OK)
    {
        delete fp;
        return NULL;
    }

    return fp;
}

void IO_Close_File(void *handle)
{
    auto fp = (FIL *)handle;
    f_close(fp);
    delete fp;
}

bool IO_Read_File(void *handle, void *buffer, size_t count, size_t &actual_read)
{
    auto fp = (FIL *)handle;
    UINT read;
    if(f_read(fp, buffer, count, &read) != FR_OK)
        return false;

    actual_read = read;
    return true;
}

bool IO_Write_File(void *handle, const void *buffer, size_t count, size_t &actual_written)
{
    auto fp = (FIL *)handle;
    UINT written;
    if(f_write(fp, buffer, count, &written) != FR_OK)
        return false;

    actual_written = written;
    return true;
}

size_t IO_Seek_File(void *handle, size_t offset, int origin)
{
    auto fp = (FIL *)handle;
    if(origin == SEEK_SET)
        f_lseek(fp, offset);
    else if(origin == SEEK_CUR)
        f_lseek(fp, f_tell(fp) + offset);
    else if(origin == SEEK_END)
        f_lseek(fp, f_size(fp) + offset);
       
    return f_tell(fp);
}

size_t IO_Get_File_Size(void *handle)
{
    auto fp = (FIL *)handle;
    return f_size(fp);
}

bool IO_Delete_File(const char *filename)
{
    return f_unlink(filename) == FR_OK;
}

static void Update_Find_Result(FindFileState &state, FILINFO &info)
{
    if(state.name)
        delete[] state.name;

    state.name = new char[strlen(info.fname) + 1];
    strcpy((char *)state.name, info.fname);

    // I realise after writing this that we don't have the RTC set up...
    tm time{};

    time.tm_sec = (info.ftime & 0x1F) * 2;
    time.tm_min = (info.ftime >> 5) & 0x3F;
    time.tm_hour = info.ftime >> 11;

    time.tm_mday = info.fdate & 0x1F;
    time.tm_mon = ((info.fdate >> 5) & 0xF) - 1;
    time.tm_year = (info.fdate >> 9) + 80; // + 1980 - 1900

    state.mod_time = mktime(&time);
}

bool Find_First_File(const char *path_glob, FindFileState &state)
{
    DIR *dir = new DIR;
    FILINFO info;

    int res = f_findfirst(dir, &info, "", path_glob);
    if(res != FR_OK || !info.fname[0])
    {
        if(res == FR_OK)
            f_closedir(dir);

        delete dir;
        return false;
    }

    state.data = dir;
    state.name = NULL;

    Update_Find_Result(state, info);

    return true;
}

bool Find_Next_File(FindFileState &state)
{
    auto dir = (DIR *)state.data;
    FILINFO info;

    if(f_findnext(dir, &info) != FR_OK || !info.fname[0])
    {
        End_Find_File(state);
        return false;
    }

    Update_Find_Result(state, info);
    return true;
}

void End_Find_File(FindFileState &state)
{
    auto dir = (DIR *)state.data;

    if(state.name)
    {
        delete[] state.name;
        state.name = NULL;
    }
    if(dir)
    {
        f_closedir(dir);
        delete dir;
        state.data = NULL;
    }
}

uint64_t Disk_Space_Available()
{
    DWORD free_clusters = 0;
    FATFS *fs;
    if(f_getfree("", &free_clusters, &fs) != FR_OK)
        return 0;

    return uint64_t(free_clusters) * fs->csize * 512;
}
