#pragma once
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>
template <typename T>
static void lock_val(const T &value, const char *path)
{
    if (access(path, F_OK) != 0) [[unlikely]] {
        // LOG("Warning: ", path, " 不存在");
        return;
    }
    umount(path);

    // chown(TmpPath,0,0);
    chmod(path, 0666);

    FILE *target_file = fopen(path, "w");
    if (!target_file) [[unlikely]] {
        // std::cerr << "无法打开文件进行写操作: " << path << std::endl;
        return;
    }

    fprintf(target_file, "%d", value);
    fclose(target_file);

    chmod(path, 0555);
}
