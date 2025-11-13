#ifndef _GB28181_TOOLS_H_
#define _GB28181_TOOLS_H_

#include <string>
#include "manscdp.h"
#include "device.h"

namespace gb28181 {

//对象转换
DeviceBase GetDeviceBase(const itemType& item);

Device GetDevice(const Catalog& catalog);

} // namespace gb28181

#endif // _GB28181_TOOLS_H_
