#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <string>
#if 0
    基于shadow3aaa的版本，微调
    使用说明：直接接收getTopApp()函数的返回值即可获取包名
    例如：std::string TopApp = getTopApp();
#endif
auto getTopAppShell() -> std::string;
bool getStringValue(const char *need_read, std::string &value);
bool getIntValue(const char *need_read, int &value);
constexpr char TopAppPidPath[] = "/sys/kernel/gbe/gbe2_fg_pid";

auto execCmdSync(const char *command) -> std::string
{
    // 执行命令并获取输出
    FILE *pipe = popen(command, "r");
    if (pipe == nullptr) [[unlikely]] {
        return {};
    }
    char buffer[2];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

auto getTopApp() -> std::string
{
    if (access(TopAppPidPath, F_OK)) [[unlikely]] {
        // printf("路径不存在使用shell度报名");
        return getTopAppShell();
    }

    int pidInt = 0;
    if (!getIntValue(TopAppPidPath, pidInt)) [[unlikely]] {
        // printf("获取pid错误");
        chmod(TopAppPidPath, 0666);
        return getTopAppShell();
    }

    if (pidInt == 0) [[unlikely]] {
        // printf("获取到pid为0");
        return getTopAppShell();
    }
    std::string pid = std::to_string(pidInt);
    std::string name;
    char cmdline[20];

    sprintf(cmdline, "/proc/%s/cmdline", pid.c_str());
    // printf("路径的cmdline是: %s\n", cmdlinePath);
    // printf("sprintf的cmdline: %s", cmdline);
    if (!getStringValue(cmdline, name)) [[unlikely]] {
        chmod(("/proc/" + pid + "/cmdline").c_str(), 0666);
        return getTopAppShell();
    }
    return name;
    //  return checkSymbol(name);
}
// 这个方式开销较大
// execCmdSync("/system/bin/dumpsys", {"window", "visible-apps"});

auto getTopAppShell() -> std::string
{
    // printf("\n使用shell读取包名\n");
    std::string name = execCmdSync("dumpsys activity lru");
    const auto pkgPos = name.find(" TOP") + 4;
    // find第二个参数:从指定的位置开始搜索
    name = name.substr(pkgPos, name.find('/', pkgPos) - pkgPos);
    size_t pos;
    if ((pos = name.find(":")) != std::string::npos) [[likely]] {
        name.erase(0, pos + 1);  // 删除冒号及其前面的内容
    }

    return name;
    // return checkSymbol(name);
}
