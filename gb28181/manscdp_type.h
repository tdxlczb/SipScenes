#ifndef _GB28181_MANSCDP_TYPE_H_
#define _GB28181_MANSCDP_TYPE_H_

#include <string>

/*
* 类型定义
* manscdp类型和字段名称完全按照gb28181文档来
* 报文映射的类型和结构体都使用小驼峰命名法
* 报文映射的字段名称都使用大驼峰命名法
*/

namespace gb28181 {

//================================================
// 命令类别
using ManscdpType = std::string;
const ManscdpType kControl = "Control";   //表示一个控制的动作
const ManscdpType kQuery = "Query";       //表示一个查询的动作
const ManscdpType kNotify = "Notify";     //表示一个通知的动作
const ManscdpType kResponse = "Response"; //表示一个请求动作的应答


//================================================
// 数据类型

// 设备编码类型
// 在取值为行政区划时可为2、4、6、8位,其他情况取值为20位
using deviceIDType = std::string;

// 命令序列号类型, 最小值为1
using SNType = int;

// 状态类型
using statusType = std::string;
const statusType kStatusON = "ON";
const statusType kStatusOFF = "OFF";

// 结果类型
using resultType = std::string;
const resultType kResultOK = "OK";
const resultType kResultERROR = "ERROR";

// 控制码类型
// 长度为8
using PTZType = std::string;

// 录像控制类型
using recordType = std::string;
const recordType kStartRecord = "Record";
const recordType kStopRecord = "StopRecord";

// 布防/撤防控制类型
using guardType = std::string;
const guardType kSetGuard = "SetGuard";
const guardType kResetGuard = "ResetGuard";

// 时间格式
using dateTimeType = std::string;

// 设备目录项类型
struct itemType
{
    // 设备/区域/系统编码(必选)
    deviceIDType DeviceID;
    // 设备/区域/系统名称(必选)
    std::string Name;
    // 当为设备时,设备厂商(必选)
    std::string Manufacturer;
    // 当为设备时,设备型号(必选)
    std::string Model;
    // 当为设备时,设备归属(必选)
    std::string Owner;
    // 行政区域(必选)
    std::string CivilCode;
    // 警区(可选)
    std::string Block;
    // 当为设备时,安装地址(必选)
    std::string Address;
    // 当为设备时,是否有子设备(必选) 默认为0; 1:有; 0:没有
    int Parental = 0;
    // 父设备/区域/系统ID(必选)
    std::string ParentID;
    // 信令安全模式(可选) 默认为0; 0:不采用; 2:S/MIME签名方式; 3:S/MIME加密签名同时采用方式; 4:数字摘要方式
    int SafetyWay = 0;
    // 注册方式(必选) 缺省为1; 1:符合IETFRFC3261标准的认证注册模式; 2:基于口令的双向认证注册模式; 3:基于数字证书的双向认证注册模式
    int RegisterWay = 1;
    // 证书序列号(有证书的设备必选)
    std::string CertNum;
    // 证书有效标识(有证书的设备必选) 缺省为0; 证书有效标识: 0:无效 1:有效
    int Certifiable = 0;
    // 无效原因码(有证书且证书无效的设备必选)
    int ErrCode = 1;
    // 证书终止有效期(有证书的设备必选)
    dateTimeType EndTime;
    // 保密属性(必选)缺省为0; 0:不涉密; 1:涉密
    int Secrecy = 0;
    // 设备/区域/系统IP地址(可选)
    std::string IPAddress;
    // 设备/区域/系统端口(可选)
    int Port = 0;
    // 设备口令(可选)
    std::string Password;
    // 设备状态(必选)
    statusType Status = kStatusOFF;


    // 还有很多业务数据元素，后续待补充
};

//文件目录项类型
struct itemFileType
{
    // 设备/区域编码(必选)
    deviceIDType DeviceID;
    // 设备/区域名称(必选)
    std::string Name;
    // 文件路径名 (可选)
    std::string FilePath;
    // 录像地址(可选)
    std::string Address;
    // 录像开始时间(可选)
    dateTimeType StartTime;
    // 录像结束时间(可选)
    dateTimeType EndTime;
    // 保密属性(必选)缺省为0; 0:不涉密; 1:涉密
    int Secrecy = 0;
    // 录像产生类型(可选) time 或 alarm 或 manual
    std::string Type;
    // 录像触发者ID(可选)
    std::string RecorderID;
    // 录像文件大小,单位:Byte(可选),这里的类型文档上就是字符串
    std::string FileSize;
};

// 在线类型
using onlineType = std::string;
const onlineType kOnline = "ONLINE";
const onlineType kOffline = "OFFLINE";

// 命令类型
using cmdType = std::string;
// 设备状态
const cmdType kDeviceStatus = "DeviceStatus";
// 设备目录
const cmdType kCatalog = "Catalog";
// 设备信息
const cmdType kDeviceInfo = "DeviceInfo";
// 文件目录检索
const cmdType kRecordInfo = "RecordInfo";
// 状态消息
const cmdType kKeepalive = "Keepalive";
// 设备控制
const cmdType kDeviceControl = "DeviceControl";
// 报警通知
const cmdType kAlarm = "Alarm";



} // namespace gb28181

#endif // _GB28181_MANSCDP_TYPE_H_
