#ifndef _GB28181_DEVICE_H_
#define _GB28181_DEVICE_H_

#include <string>
#include <map>
#include "device_base.h"
#include "device_channel.h"

namespace gb28181 {

// 设备
struct Device : public DeviceBase
{
    Device() = default;
    Device(const DeviceBase& base) : DeviceBase(base) {}

    int channelNum = 0;
    std::map<std::string, DeviceChannel> channels;
};


} // namespace gb28181

#endif // _GB28181_DEVICE_H_
