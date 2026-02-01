#include <stdio.h>
#include <string.h>

#include "file.h"

#include "config.h"

#include "fatfs/ff.h"

const uint8_t *IO_Get_Appended_File(const char *name, uint32_t &size)
{
    // find end of binary
    extern char __flash_binary_end;
    auto appFilesPtr = &__flash_binary_end;
    appFilesPtr = (char *)(((uintptr_t)appFilesPtr) + 0xFFF & ~0xFFF); // round up to 4k boundary

    // check for appended files
    if(memcmp(appFilesPtr, "APPFILES", 8) != 0)
        return nullptr;

    // loop through files
    uint32_t numFiles = *reinterpret_cast<uint32_t *>(appFilesPtr + 8);

    const int headerSize = 12, fileHeaderSize = 8;

    auto dataPtr = appFilesPtr + headerSize + fileHeaderSize * numFiles;

    for(auto i = 0u; i < numFiles; i++)
    {
        auto filenameLength = *reinterpret_cast<uint16_t *>(appFilesPtr + headerSize + i * fileHeaderSize);
        auto fileLength = *reinterpret_cast<uint32_t *>(appFilesPtr + headerSize + i * fileHeaderSize + 4);

        // check for name match
        if(strncmp(name, dataPtr, filenameLength) == 0)
        {
            size = fileLength;
            return reinterpret_cast<const uint8_t *>(dataPtr + filenameLength);
        }

        dataPtr += filenameLength + fileLength;
    }

    return nullptr;
}

static bool IO_Open_Appended_File(const char *filename, FIL *fp)
{
    uint32_t size;
    auto ptr = IO_Get_Appended_File(filename, size);

    if(!ptr)
        return false;

    // store relevant data into file pointer
    fp->obj.fs = nullptr;
    fp->obj.id = 0xAAAA;
    fp->obj.objsize = size;
    fp->fptr = 0;
    fp->sect = (uint32_t)ptr;

    return true;
}

static bool IO_Is_App_File_Ptr(FIL *fp)
{
    return fp && fp->obj.fs == nullptr && fp->obj.id == 0xAAAA;
}

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
        // try flash appended files
        if(mode == READ && IO_Open_Appended_File(filename, fp))
            return fp;

        delete fp;
        return NULL;
    }

    return fp;
}

void IO_Close_File(void *handle)
{
    auto fp = (FIL *)handle;
    if(!IO_Is_App_File_Ptr(fp))
        f_close(fp);
    delete fp;
}

bool IO_Read_File(void *handle, void *buffer, size_t count, size_t &actual_read)
{
    auto fp = (FIL *)handle;
    if(IO_Is_App_File_Ptr(fp))
    {
        // copy data from flash
        auto ptr = (const uint8_t *)fp->sect;
        auto offset = fp->fptr;
        auto size = fp->obj.objsize;

        if(offset + count > size)
            count = size - offset;

        memcpy(buffer, ptr + offset, count);

        fp->fptr += count;

        actual_read = count;
    }
    else
    {
        // real file
        UINT read;
        if(f_read(fp, buffer, count, &read) != FR_OK)
            return false;

        actual_read = read;
    }

    return true;
}

bool IO_Write_File(void *handle, const void *buffer, size_t count, size_t &actual_written)
{
    auto fp = (FIL *)handle;

    // not writable
    if(IO_Is_App_File_Ptr(fp))
        return false;

    UINT written;
    if(f_write(fp, buffer, count, &written) != FR_OK)
        return false;

    actual_written = written;
    return true;
}

size_t IO_Seek_File(void *handle, size_t offset, int origin)
{
    auto fp = (FIL *)handle;

    if(origin == SEEK_CUR)
        offset = f_tell(fp) + offset;
    else if(origin == SEEK_END)
        offset = f_size(fp) + offset;

    if(IO_Is_App_File_Ptr(fp))
    {
        // manual clamp
        if(offset > f_size(fp))
            offset = f_size(fp);

        fp->fptr = offset;
    }
    else
        f_lseek(fp, offset);
       
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
#ifdef NO_SD_CARD
    // fake enough to start the game
    return 4096 * 1024;
#else
    DWORD free_clusters = 0;
    FATFS *fs;
    if(f_getfree("", &free_clusters, &fs) != FR_OK)
        return 0;

    return uint64_t(free_clusters) * fs->csize * 512;
#endif
}
