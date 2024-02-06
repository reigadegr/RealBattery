#include <stdio.h>
#include <sys/stat.h>

#include <array>
#include <string>

bool getIntValue(const char *need_read, int &value)
{
    FILE *file = fopen(need_read, "r");
    if (file == nullptr) [[unlikely]] {
        chmod(need_read, 0444);
        return false;
    }

    fscanf(file, "%d", &value);
    fclose(file);
    return true;
}

bool getFloatValue(const char *need_read, float &value)
{
    FILE *file = fopen(need_read, "r");
    if (file == nullptr) [[unlikely]] {
        chmod(need_read, 0444);
        return false;
    }
    fscanf(file, "%f", &value);
    fclose(file);
    return true;
}
