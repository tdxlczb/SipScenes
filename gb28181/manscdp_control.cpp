#include "manscdp_control.h"

namespace gb28181 {

static std::string BuildManscdpControl(const ManscdpBase& manscdp, const std::string& control)
{
    std::string xml;
    xml.append("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
    xml.append("<Control>\r\n");
    xml.append("<CmdType>" + manscdp.CmdType + "</CmdType>\r\n");
    xml.append("<SN>" + std::to_string(manscdp.SN) + "</SN>\r\n");
    xml.append("<DeviceID>" + manscdp.DeviceID + "</DeviceID>\r\n");
    xml.append(control);
    xml.append("</Control>\r\n");
    return xml;
}

std::string BuildPTZCmdControl(const ControlDeviceControl& manscdp)
{
    std::string control;
    control.append("<PTZCmd>" + manscdp.PTZCmd.ToString() + "</PTZCmd>\r\n");
    return BuildManscdpControl(manscdp, control);
}

std::string BuildTeleBootControl(const ControlDeviceControl& manscdp)
{
    std::string control;
    if (manscdp.TeleBoot == "Boot") {
        control.append("<TeleBoot>Boot</TeleBoot>\r\n");
    }
    return BuildManscdpControl(manscdp, control);
}

std::string BuildRecordCmdControl(const ControlDeviceControl& manscdp)
{
    std::string control;
    control.append("<RecordCmd>" + manscdp.RecordCmd + "</RecordCmd>\r\n");
    return BuildManscdpControl(manscdp, control);
}

std::string BuildGuardCmdControl(const ControlDeviceControl& manscdp)
{
    std::string control;
    control.append("<GuardCmd>" + manscdp.GuardCmd + "</GuardCmd>\r\n");
    return BuildManscdpControl(manscdp, control);
}

std::string BuildAlarmCmdControl(const ControlDeviceControl& manscdp)
{
    std::string control;
    if (manscdp.AlarmCmd == "ResetAlarm") {
        control.append("<AlarmCmd>ResetAlarm</AlarmCmd>\r\n");
    }
    return BuildManscdpControl(manscdp, control);
}

std::string BuildIFameCmdControl(const ControlDeviceControl& manscdp)
{
    std::string control;
    if (manscdp.IFameCmd == "Send") {
        control.append("<IFameCmd>Send</IFameCmd>\r\n");
    }
    return BuildManscdpControl(manscdp, control);
}

std::string BuildDragZoomInControl(const ControlDeviceControl& manscdp)
{
    std::string control;
    control.append("<DragZoomIn>\r\n");
    control.append("<Length>" + std::to_string(manscdp.DragZoomIn.Length) + "</Length>\r\n");
    control.append("<Width>" + std::to_string(manscdp.DragZoomIn.Width) + "</Width>\r\n");
    control.append("<MidPointX>" + std::to_string(manscdp.DragZoomIn.MidPointX) + "</MidPointX>\r\n");
    control.append("<MidPointY>" + std::to_string(manscdp.DragZoomIn.MidPointY) + "</MidPointY>\r\n");
    control.append("<LengthX>" + std::to_string(manscdp.DragZoomIn.LengthX) + "</LengthX>\r\n");
    control.append("<LengthY>" + std::to_string(manscdp.DragZoomIn.LengthY) + "</LengthY>\r\n");
    control.append("</DragZoomIn>\r\n");
    return BuildManscdpControl(manscdp, control);
}

std::string BuildDragZoomOutControl(const ControlDeviceControl& manscdp)
{
    std::string control;
    control.append("<DragZoomOut>\r\n");
    control.append("<Length>" + std::to_string(manscdp.DragZoomOut.Length) + "</Length>\r\n");
    control.append("<Width>" + std::to_string(manscdp.DragZoomOut.Width) + "</Width>\r\n");
    control.append("<MidPointX>" + std::to_string(manscdp.DragZoomOut.MidPointX) + "</MidPointX>\r\n");
    control.append("<MidPointY>" + std::to_string(manscdp.DragZoomOut.MidPointY) + "</MidPointY>\r\n");
    control.append("<LengthX>" + std::to_string(manscdp.DragZoomOut.LengthX) + "</LengthX>\r\n");
    control.append("<LengthY>" + std::to_string(manscdp.DragZoomOut.LengthY) + "</LengthY>\r\n");
    control.append("</DragZoomOut>\r\n");
    return BuildManscdpControl(manscdp, control);
}

} // namespace gb28181