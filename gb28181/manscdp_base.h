#ifndef _GB28181_MANSCDP_BASE_H_
#define _GB28181_MANSCDP_BASE_H_

#include "manscdp_type.h"

namespace gb28181 {

struct ManscdpBase
{
    // 命令类型
    cmdType CmdType;
    // 命令序列号, 默认为1
    SNType  SN = 1;
    // 目标设备的设备/区域、联网系统编码
    deviceIDType DeviceID;
};

} // namespace gb28181

#endif // _GB28181_MANSCDP_BASE_H_
