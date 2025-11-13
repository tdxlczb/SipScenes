#include "tools.h"

namespace gb28181 {

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
    device.status = item.Status;
    return device;
}

Device GetDevice(const Catalog& catalog)
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