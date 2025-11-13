#ifndef _GB28181_DEVICE_BASE_H_
#define _GB28181_DEVICE_BASE_H_

#include <string>

namespace gb28181 {

struct DeviceBase
{
    // 设备国标编号
    std::string deviceId;

    // 设备名称
    std::string name;

    // 设备厂商
    std::string manufacturer;

    // 设备型号
    std::string model;

    // 固件版本
    std::string firmware;

    // 设备归属
    std::string owner;

    // 行政区域
    std::string civilCode;

    // 设备地址
    std::string address;

    // 是否有子设备
    int parental = 0;

    // 父结点编号
    std::string parentId;

    // 信令安全模式
    int safetyWay = 0;

    // 注册方式，默认为1
    int registerWay = 1;

    // 保密属性
    int secrecy = 0;

    // 设备ip地址
    std::string ip;

    // 设备端口
    int port = 0;

    // 设备状态：0表示OFF，1表示ON
    int status = 0;
};

} // namespace gb28181

#endif // _GB28181_DEVICE_BASE_H_
