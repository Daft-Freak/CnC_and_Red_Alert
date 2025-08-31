#include <dirent.h>
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

// need to keep ref to orig path
struct DirState
{
    DIR *dir;
    char *name_glob;
};

static bool Update_Find_Result(FindFileState &state)
{
    auto state_data = (DirState *)state.data;

    dirent *ent;
    struct stat stat_buf;

    auto glob = strchr(state_data->name_glob, '*');
    int pre_len = glob - state_data->name_glob;
    auto post_str = glob + 1;
    int post_len = strlen(post_str);
    
    while((ent = readdir(state_data->dir)))
    {
        // check prefix
        if(strncmp(ent->d_name, state_data->name_glob, pre_len) != 0)
            continue;

        // check suffix
        if(strncmp(ent->d_name + strlen(ent->d_name) - post_len, post_str, post_len) != 0)
            continue;

        char prefixed_path[300];
        snprintf(prefixed_path, sizeof(prefixed_path), "%s%s", path_prefix, ent->d_name);

        // check if dir (or stat fails)
        if(stat(prefixed_path, &stat_buf) != 0 || S_ISDIR(stat_buf.st_mode))
            continue;

        state.name = ent->d_name;
        state.mod_time = stat_buf.st_mtim.tv_sec;
        return true;
    }

    return false;
}

bool Find_First_File(const char *path_glob, FindFileState &state)
{
    // the only uses in the game are simple file globs (*.x, x.*, x*.y)
    if(strchr(path_glob, '/') || strchr(path_glob, '\\'))
        return false;

    if(!strchr(path_glob, '*') || strchr(path_glob, '*') != strrchr(path_glob, '*'))
        return false;

    // open
    DIR *dir = opendir(path_prefix);

    if(!dir)
        return false;

    // need to store two pointers, yay
    auto state_data = new DirState;
    state_data->dir = dir;
    state_data->name_glob = new char[strlen(path_glob) + 1];
    strcpy(state_data->name_glob, path_glob);

    state.data = state_data;
    state.name = nullptr;

    if(!Update_Find_Result(state))
    {
        closedir(dir);
        delete state_data->name_glob;
        delete state_data;
        return false;
    }

    return true;
}

bool Find_Next_File(FindFileState &state)
{
    if(!Update_Find_Result(state))
    {
        End_Find_File(state);
        return false;
    }
    return true;
}

void End_Find_File(FindFileState &state)
{
    if(state.data)
    {
        auto state_data = (DirState *)state.data;
        closedir(state_data->dir);
        delete state_data->name_glob;
        delete state_data;
        state.data = nullptr;
    }
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
