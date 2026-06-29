#include "tools.h"
#include <atomic>
#include <chrono>

namespace gb28181 {

//str转int
bool str2int(const std::string& s, int& out)
{
    char* end = nullptr;
    errno = 0;
    long val = std::strtol(s.c_str(), &end, 10);

    // 1. 空串 / 无数字
    if (end == s.c_str()) return false;
    // 2. 后缀非法字符
    if (*end != '\0')       return false;
    // 3. 越界（long 转 int）
    if (errno == ERANGE || val < INT_MIN || val > INT_MAX) return false;

    out = static_cast<int>(val);
    return true;
}

// 线程安全、进程级单调递增
int GetSN() {
    static std::atomic<uint64_t> seq{ 0 };
    // 毫秒时间戳取低 32 位 + 自增 24 位 → 最多 56 位，字符串长度 <= 17
    uint64_t t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    uint64_t s = seq.fetch_add(1, std::memory_order_relaxed);
    return (t & 0xFFFFFFFF) * 1000000 + (s & 0xFFFFFF);
}

//对象转换
DeviceBase GetDeviceBase(const itemType& item)
{
    DeviceBase device;
    device.deviceId = item.DeviceID;
    device.name = item.Name;
    device.manufacturer = item.Manufacturer;
    device.model = item.Model;
    device.owner = item.Owner;
    device.civilCode = item.CivilCode;
    device.address = item.Address;
    device.parental = item.Parental;
    device.parentId = item.ParentID;
    device.safetyWay = item.SafetyWay;
    device.registerWay = item.RegisterWay;
    device.secrecy = item.Secrecy;
    device.ip = item.IPAddress;
    device.port = item.Port;
    device.status = (item.Status == kStatusON) ? 1 : 0;
    return device;
}

Device GetDevice(const ResponseCatalog& catalog)
{
    Device device;
    device.deviceId = catalog.DeviceID;
    device.channelNum = catalog.SumNum;
    for (size_t i = 0; i < catalog.DeviceList.size(); i++)
    {
        auto base = GetDeviceBase(catalog.DeviceList[i]);
        if (!base.deviceId.empty()) {
            device.channels.emplace(base.deviceId, base);
        }
    }
    return device;
}

} // namespace gb28181