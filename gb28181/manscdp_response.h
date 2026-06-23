#ifndef _GB28181_MANSCDP_RESPONSE_H_
#define _GB28181_MANSCDP_RESPONSE_H_

#include "manscdp_base.h"
#include <vector>

namespace gb28181 {

using ResponseBase = ManscdpBase;

//==================== DeviceControl ====================
// 设备控制应答
struct ResponseDeviceControl :public ResponseBase
{
    // 执行结果标志
    resultType Result = kResultERROR;
};
//==================== DeviceControl ====================


//==================== Alarm ====================
// 报警通知应答
struct ResponseAlarm :public ResponseBase
{
    // 执行结果标志
    resultType Result = kResultERROR;
};
//==================== Alarm ====================


//==================== Catalog ====================
// 设备目录信息查询应答
struct ResponseCatalog : public ResponseBase
{
    // 查询结果总数(必选)
    int SumNum = 0;
    // 设备目录项列表,Num 表示目录项个数
    std::vector<itemType> DeviceList;
    // 执行结果标志
    resultType Result = kResultERROR;
};
//==================== Catalog ====================


//==================== DeviceInfo ====================
//设备信息查询应答
struct ResponseDeviceInfo : public ManscdpBase
{
    // 目标设备/区域/系统的名称(可选)
    std::string DeviceName;
    // 查询结果(必选)
    resultType Result = kResultERROR;
    // 设备生产商(可选)
    std::string Manufacturer;
    // 设备型号(可选)
    std::string Model;
    // 设备固件版本(可选)
    std::string Firmware;
    // 视频输入通道数(可选)
    int Channel = 0;
};
//==================== DeviceInfo ====================


//==================== DeviceStatus ====================
// 设备状态信息查询应答
struct ResponseDeviceStatus : public ResponseBase
{
    // 查询结果标志(必选)
    resultType Result = kResultERROR;
    // 是否在线(必选)
    onlineType Online = kOffline;
    // 是否正常工作(必选)
    resultType Status = kResultERROR;
    // 不正常工作原因(可选)
    std::string Reason;
    // 是否编码(可选)
    statusType Encode = kStatusOFF;
    // 是否录像(可选)
    statusType Record = kStatusOFF;
    // 设备时间和日期(可选)
    dateTimeType dateTime;
    // 还有一些待补充
};
//==================== DeviceStatus ====================


//==================== RecordInfo ====================
//文件目录检索应答
struct ResponseRecordInfo : public ResponseBase
{
    // 设备/区域名称(必选)
    std::string Name;
    // 查询结果总数(必选)
    int SumNum = 0;
    // 文件目录项列表
    std::vector<itemType> RecordList;
};
//==================== RecordInfo ====================


ResponseCatalog GetResponseCatalog(const std::string& xml);


} // namespace gb28181

#endif // _GB28181_MANSCDP_RESPONSE_H_
