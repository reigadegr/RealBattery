#include <stdio.h>
#include <sys/stat.h>

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

bool getStringValue(const char *need_read, std::string &value)
{
    FILE *pipe = fopen(need_read, "r");
    if (pipe == nullptr) [[unlikely]] {
        chmod(need_read, 0444);
        return false;
    }

    char buffer[13];
    value = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        pclose(pipe);
        value += buffer;
    }
    return true;
}
