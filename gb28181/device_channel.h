#ifndef _GB28181_DEVICE_CHANNEL_H_
#define _GB28181_DEVICE_CHANNEL_H_

#include <string>
#include "device_base.h"

namespace gb28181 {

// 设备通道
struct DeviceChannel : public DeviceBase
{
    DeviceChannel() = default;
    DeviceChannel(const DeviceBase& base) : DeviceBase(base) {}

};

}
#endif // _GB28181_DEVICE_CHANNEL_H_
