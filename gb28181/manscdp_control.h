#ifndef _GB28181_MANSCDP_CONTROL_H_
#define _GB28181_MANSCDP_CONTROL_H_

#include "manscdp_base.h"

namespace gb28181 {

// 拉框控制
struct dragZoomType
{
    // 播放窗口长度像素值(必选)
    int Length = 0;
    // 播放窗口宽度像素值(必选)
    int Width = 0;
    // 拉框中心的横轴坐标像素值(必选)
    int MidPointX = 0;
    // 拉框中心的纵轴坐标像素值(必选)
    int MidPointY = 0;
    // 拉框长度像素值(必选)
    int LengthX = 0;
    // 拉框宽度像素值(必选)
    int LengthY = 0;
};

struct ControlBase : public ManscdpBase
{
    ControlBase() {
        ManscdpType = kControl;
    };
};

//==================== DeviceControl ====================
// 设备控制
struct ControlDeviceControl : public ControlBase
{
    // 球机/云台控制命令(可选,控制码应符合附录 A 中的 A.3中的规定)
    PTZType PTZCmd;
    // 远程启动控制命令(可选),唯一合法值"Boot"
    std::string TeleBoot;
    // 录像控制命令(可选)
    recordType RecordCmd;
    // 报警布防/撤防命令(可选)
    guardType GuardCmd;
    // 报警复位命令(可选),唯一合法值"ResetAlarm"
    std::string AlarmCmd;
    // 强制关键帧命令,设备收到此命令应立刻发送一个IDR帧(可选),唯一合法值"Send"
    std::string IFameCmd;
    // 拉框放大控制命令(可选)
    dragZoomType DragZoomIn;
    // 拉框缩小控制命令(可选)
    dragZoomType DragZoomOut;
    // 还有其他控制命令，待补充

    // 上述所有命令互斥，只能同时出现一种命令
};
//==================== DeviceControl ====================


struct basicParamType
{
    // 设备名称(可选)
    std::string Name;
    // 注册过期时间(可选)
    int Expiration = 0;
    // 心跳间隔时间(可选)
    int HeartBeatInterval = 0;
    // 心跳超时次数(可选)
    int HeartBeatCount = 0;
};

//==================== DeviceConfig ====================
// 设备配置
struct ControlDeviceConfig : public ControlBase
{
    // 基本参数配置(可选)
    basicParamType BasicParam;

    // 还有其他控制命令，待补充
};
//==================== DeviceConfig ====================


std::string BuildPTZCmdControl(const ControlDeviceControl& manscdp);
std::string BuildTeleBootControl(const ControlDeviceControl& manscdp);
std::string BuildRecordCmdControl(const ControlDeviceControl& manscdp);
std::string BuildGuardCmdControl(const ControlDeviceControl& manscdp);
std::string BuildAlarmCmdControl(const ControlDeviceControl& manscdp);
std::string BuildIFameCmdControl(const ControlDeviceControl& manscdp);
std::string BuildDragZoomInControl(const ControlDeviceControl& manscdp);
std::string BuildDragZoomOutControl(const ControlDeviceControl& manscdp);

} // namespace gb28181

#endif // _GB28181_MANSCDP_CONTROL_H_
