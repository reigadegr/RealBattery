#include <pthread.h>
#include <stdio.h>

#include <algorithm>
#include <array>
#include <fstream>
// #include <mutex>
#include <thread>

#include "LockValue.h"
constexpr float range = 3.7 - 3.4;

constexpr char voltage_path[] = "/sys/class/power_supply/battery/voltage_now";

constexpr char target_path[] = "/sys/class/power_supply/bms/capacity";

constexpr char need_read_path[] = "/sys/class/power_supply/bms/capacity_raw";

constexpr char ChargingStatus_Path[] = "/sys/class/power_supply/battery/status";

constexpr char charge_current_Path[] =
    "/sys/class/power_supply/battery/constant_charge_current";

constexpr char CloseAppCmd[] =
    "nohup am start com.miui.home/com.miui.home.launcher.Launcher >/dev/null "
    "2>&1 &";

constexpr ::std::array WhiteList{
    "com.miui.home",        "bin.mt.plus",        "com.omarea.vtools",
    "com.tencent.mobileqq", "com.tencent.mm",     "com.android.settings",
    "com.android.contacts", "com.android.camera", "com.android.mms"};

// int min_value = 101;
bool isCharging = false;
auto getTopApp() -> std::string;

bool getFloatValue(const char *need_read, float &value);
bool getIntValue(const char *need_read, int &value);
bool getStringValue(const char *need_read, std::string &value);

static inline int getCapacity(const float &volate_percentage)
{
    return volate_percentage * 10;
}
// static std::mutex confMutex;
static inline void appCloser(const int &low_capacity, const int &capacity)
{
    // {
    // std::lock_guard<std::mutex> lock(confMutex);
    if (low_capacity < capacity) {
        const std::string TopApp = getTopApp();
        for (const auto &app : WhiteList) {
            // 如果当前app在白名单，则不关闭
            if (TopApp.find(app) != std::string::npos) {
                // printf("白名单，返回\n");
                return;
            }
        }
        // std::cout << "关闭\n";
        lock_val(11725000, charge_current_Path);
        // printf("包名: %s关闭\n", TopApp.c_str());
        system(CloseAppCmd);
    }
    // }
    return;
}

static inline void heavyThread()
{
    pthread_setname_np(pthread_self(), "HeavyThread");
    // printf("重负载线程开始运行开始\n");
    // static std::mutex confMutex;

    /*
    const char *target_path = ((const char **)arg)[0];
    const char *read = ((const char **)arg)[1];
    */
    int highPercentage = 0;
    float voltage_value = 0.0;

    while (true) {
        // const std::string TopApp = getTopApp();
        // printf("包名: -%s-\n", TopApp.c_str());
        if (!getIntValue(need_read_path, highPercentage)) [[unlikely]] {
            // printf("heavy线程无法读取needread\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }
        if (isCharging || highPercentage > 1000) {
            // printf("充电或电量大10通过读节点写入电量\n");
            highPercentage = highPercentage / 100;
            lock_val(highPercentage, target_path);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        // 百分比小于等于10

        if (!getFloatValue(voltage_path, voltage_value)) [[unlikely]] {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }
        // printf("使用读取电压写电量\n");
        voltage_value = voltage_value / 1000000;

        // 这里电压值变成剩余百分比了
        voltage_value = (voltage_value - 3.4) / range;

        //  获取低电量值
        int low_capacity = getCapacity(voltage_value);

        low_capacity = std::clamp(low_capacity, 0, 10);

        // printf("百分比是: %f\n", voltage_value);
        // printf("低电量值为: %d\n", low_capacity);
        // printf("最小值: %d\n", min_value);
        // {
        // std::lock_guard<std::mutex> lock(confMutex);
        // if (low_capacity < min_value) {
        // min_value = low_capacity;
        // 防止低电量充电乱跳
        // if (min_value != 101) [[likely]]
        lock_val(low_capacity, target_path);

        // 低电量， 如果当前没在桌面，就关闭当前应用
        // 通过打开桌面实现效果
        appCloser(low_capacity, 6);
        // }
        // }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

static inline void ResetMiscValue()
{
    pthread_setname_np(pthread_self(), "ResetMiscValue");
    std::string status = "";
    while (true) {
        // printf("开始\n");
        if (!getStringValue(ChargingStatus_Path, status)) [[unlikely]] {
            // printf("读取充电状态error\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }
        // printf("充电吗?-%s-\n", value.c_str());
        // 如果是充电..
        if (status.find("Discharging") == std::string::npos) {
            // std::cout << "在充电\n";
            // std::lock_guard<std::mutex> lock(confMutex);
            isCharging = true;
            // min_value = 101;
            int voltage_value = 0.0;
            if (!getIntValue(voltage_path, voltage_value)) [[unlikely]] {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }
            if (voltage_value > 4460000) {
                lock_val(0, charge_current_Path);
            }
            else if (voltage_value < 4200000) {
                lock_val(11725000, charge_current_Path);
            }
        }
        else {
            isCharging = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

static inline void ThreadGroup()
{
    pthread_setname_np(pthread_self(), "GroupThread");

    // const char *args[] = {target_path, need_read};

    // pthread_t t;
    std::thread s(&ResetMiscValue);
    // printf ("创建cpp线程对象\n");
    std::thread t(&heavyThread);
    // printf ("create重负载线程\n");
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(86400));
    }
    // s.join();
    // t.join();
}

int main(int argc, char **argv)
{
    pthread_setname_np(pthread_self(), "MainThread");
    printf("hello");
    std::thread GroupThread(&ThreadGroup);
    GroupThread.join();

    // ThreadGroup();
}
