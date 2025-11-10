#ifndef _GB28181_DEVICE_CHANNEL_H_
#define _GB28181_DEVICE_CHANNEL_H_

#include <string>

namespace gb28181 {

struct DeviceChannel
{
    // 数据库自增ID
    int id = 0;
    // 通道国标编号
    std::string channelId;
    // 设备国标编号，对应Device的deviceId，用于将通道绑定设备
    std::string deviceId;
    // 通道名称
    std::string name;

};

}
#endif // _GB28181_DEVICE_CHANNEL_H_
