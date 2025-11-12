#ifndef _GB28181_MANSCDP_H_
#define _GB28181_MANSCDP_H_

#include <string>
#include <vector>

namespace gb28181 {

/*
* 类型和成员的命名完全按照gb28181文档来
*/

//命令类别
using ManscdpType = std::string;
const ManscdpType kControl = "Control";
const ManscdpType kQuery = "Query";
const ManscdpType kNotify = "Notify";
const ManscdpType kResponse = "Response";

//命令类型
using cmdType = std::string;

//设备编码类型
using deviceIDType = std::string;

//命令序列号类型
using SNType = int; //最小值为1

//状态类型
using statusType = int;
const statusType kStatusON = 1; //字符串"ON"
const statusType kStatusOFF = 0; //字符串"OFF"

//结果类型
using resultType = int;
const resultType kResultOK = 1;
const resultType kResultERROR = 0;

//在线类型
using onlineType = int;
const onlineType kOnline = 1; //字符串"ONLINE"
const onlineType kOffline = 0; //字符串"OFFLINE"

//设备目录项类型
struct itemType
{
    deviceIDType DeviceID;
    std::string Name;
    std::string Manufacturer;
    std::string Model;
    std::string Owner;
    std::string CivilCode;
    std::string Address;
    int Parental = 0;
    std::string ParentID;
    int SafetyWay = 0;
    int RegisterWay = 0;
    int Secrecy = 0;
    std::string IPAddress;
    int Port = 0;
    statusType Status = kStatusOFF;
};

struct ManscdpBase
{
    cmdType CmdType;
    int  SN;
    deviceIDType DeviceID;
};

using QueryBase = ManscdpBase;

//==================== Catalog ====================
//设备目录查询或订阅请求 必选:SN,DeviceID  可选:StartTime,EndTime
const cmdType kCatalog = "Catalog";
struct QueryCatalog : public QueryBase
{
    std::string StartTime;
    std::string EndTime;
};
struct ResponseCatalog : public ManscdpBase
{
    int SumNum = 0;
    std::vector<itemType> DeviceList;
};
using Catalog = ResponseCatalog;
//==================== Catalog ====================

//==================== DeviceStatus ====================
//设备状态查询请求 必选:SN,DeviceID
const cmdType kDeviceStatus = "DeviceStatus";
struct QueryDeviceStatus : public QueryBase
{

};
struct ResponseDeviceStatus : public ManscdpBase
{
    resultType Result = kResultERROR;
    onlineType Online = kOffline;
    resultType Status = kResultERROR;
    std::string Reason;
    statusType Encode = kStatusOFF;
    //还有一些待补充
};
using DeviceStatus = ResponseDeviceStatus;
//==================== DeviceStatus ====================

//==================== DeviceInfo ====================
//设备信息查询请求 必选:SN,DeviceID
const cmdType kDeviceInfo = "DeviceInfo";
struct QueryDeviceInfo : public QueryBase
{

};
struct ResponseDeviceInfo : public ManscdpBase
{
};
using DeviceInfo = ResponseDeviceInfo;

//==================== DeviceInfo ====================

//==================== PresetQuery ====================
//设备预置位查询请求 必选:SN,DeviceID
const cmdType kPresetQuery = "PresetQuery";

//==================== PresetQuery ====================



struct QueryCmd : public ManscdpBase
{
    std::string StartTime;
    std::string EndTime;
};

std::string GetSN();

std::string BuildQuery(const QueryBase& manscdp);
std::string BuildQueryCatalog(const QueryCatalog& manscdp);

Catalog GetCatalog(const std::string& xml);

} // namespace gb28181

#endif // _GB28181_MANSCDP_H_
