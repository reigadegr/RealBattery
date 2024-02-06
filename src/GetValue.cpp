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
constexpr std::array endFlag{'\n', '\0'};
bool getStringValue(const char *need_read, std::string &value)
{
    FILE *pipe = fopen(need_read, "r");
    if (pipe == nullptr) [[unlikely]] {
        chmod(need_read, 0444);
        return false;
    }

    char buffer[2];
    value = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        value += buffer;
    }
    size_t pos;
    for (const auto &end : endFlag) {
        if ((pos = value.find(end)) != std::string::npos) {
            // 删除换行符号及其后面的内容
            // value.erase(pos, pos + 1);

            // 这样可以防止奇怪的多余符号
            value = value.substr(0, pos);
        }
    }

    pclose(pipe);
    return true;
}
