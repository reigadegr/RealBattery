#include <pthread.h>
#include <stdio.h>

#include <array>
#include <fstream>
#include <mutex>
#include <thread>

#include "LockValue.h"
constexpr float range = 3.7 - 3.4;

constexpr char voltage_path[] = "/sys/class/power_supply/battery/voltage_now";

constexpr char target_path[] = "/sys/class/power_supply/bms/capacity";

constexpr char need_read_path[] = "/sys/class/power_supply/bms/capacity_raw";

constexpr char CloseAppCmd[] =
    "nohup am start com.miui.home/com.miui.home.launcher.Launcher >/dev/null "
    "2>&1 &";
constexpr char ChargingStatus_Path[] = "/sys/class/power_supply/battery/status";

constexpr char charge_current_Path[] =
    "/sys/class/power_supply/battery/constant_charge_current";

constexpr std::array WhiteList{"com.miui.home", "bin.mt.plus",
                               "com.omarea.vtools", "com.tencent.mobileqq",
                               "com.tencent.mm"};

int value = 0;
int min_value = 101;
int low_capacity = 0;
float frs = 0.0;
float voltage_value = 0.0;

auto getTopApp() -> std::string;

bool getFloatValue(const char *need_read, float &value);
bool getIntValue(const char *need_read, int &value);
bool getStringValue(const char *need_read, std::string &value);

static inline int getCapacity(const float &frs) { return frs * 10; }
static inline void appCloser(const int &capacity)
{
    // std::cout << "app closer开始运行\n";
    // std::cout << "最小剩余电量百分比: " << min_value << std::endl;
    const std::string TopApp = getTopApp();
    if (min_value < capacity) {
        for (const auto &app : WhiteList) {
            // 如果当前app在白名单，则不关闭
            if (TopApp.find(app) != std::string::npos) {
                // std::cout << "白名单，返回\n";
                return;
            }
        }
        // std::cout << "关闭\n";
        lock_val(11725000, charge_current_Path);
        system(CloseAppCmd);
    }
    return;
}
static inline void *heavyThread(void *)
{
    pthread_setname_np(pthread_self(), "HeavyThread");
    static std::mutex confMutex;

    /*
    const char *target_path = ((const char **)arg)[0];
    const char *read = ((const char **)arg)[1];
    */

    while (true) {
        if (!getIntValue(need_read_path, value)) [[unlikely]] {
            continue;
        }

        // 当充电时，这里可能会导致电量百分比乱跳
        if (value < 1001) {
            if (!getFloatValue(voltage_path, voltage_value)) [[unlikely]] {
                continue;
            }

            voltage_value = voltage_value / 1000000;
            frs = (voltage_value - 3.4) / range;

            //  获取低电量值
            low_capacity = getCapacity(frs);

            std::lock_guard<std::mutex> lock(confMutex);
            if (low_capacity < min_value) {
                min_value = low_capacity;
                // 防止低电量充电乱跳
                if (min_value != 101) [[likely]] {
                    lock_val(min_value, target_path);
                }
                // 低电量， 如果当前没在桌面，就关闭当前应用
                // 通过打开桌面实现效果
                appCloser(6);
            }
        }
        else {
            value = value / 100;
            lock_val(value, target_path);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
static inline void ResetMin_value()
{
    pthread_setname_np(pthread_self(), "ResetMinValue");
    static std::mutex confMutex;
    std::string value = "";
    while (true) {
        if (!getStringValue(ChargingStatus_Path, value)) [[unlikely]] {
            continue;
        }
        // 如果是充电..
        if (value.find("Discharging") == std::string::npos) {
            // std::cout << "在充电\n";
            std::lock_guard<std::mutex> lock(confMutex);
            min_value = 101;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main(int argc, char **argv)
{
    pthread_setname_np(pthread_self(), "MainThread");

    // const char *args[] = {target_path, need_read};

    pthread_t t;
    std::thread s(ResetMin_value);
    pthread_create(&t, NULL, &heavyThread, NULL);
    pthread_join(t, NULL);
    s.join();
}
