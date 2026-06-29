#ifndef _GB28181_MANSCDP_TYPE_H_
#define _GB28181_MANSCDP_TYPE_H_

#include <string>
#include "manscdp_control_type.h"
/*
* 类型定义
* manscdp类型和字段名称完全按照gb28181文档来
* 报文映射的类型和结构体都使用小驼峰命名法
* 报文映射的字段名称都使用大驼峰命名法
*/

namespace gb28181 {

//================================================
// 命令类别
using manscdpType = std::string;
const manscdpType kControl = "Control";   //表示一个控制的动作
const manscdpType kQuery = "Query";       //表示一个查询的动作
const manscdpType kNotify = "Notify";     //表示一个通知的动作
const manscdpType kResponse = "Response"; //表示一个请求动作的应答


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
using PTZType = PTZCmdType;

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
    int ErrCode = 0;
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
    // 经度(可选)
    double Longitude = 0.0;
    // 纬度(可选)
    double Latitude = 0.0;

    // 以下是Info子节点
    // 摄像机类型扩展,标识摄像机类型:1-球机;2-半球;3-固定枪机;4-遥控枪机。当目录项为摄像机时可选
    int PTZType;
    // 摄像机位置类型扩展。1-省际检查站、2-党政机关、3-车站码头、4-中心广场、5 - 体育场馆、
    // 6 - 商业中心、7 - 宗教场所、8 - 校园周边、9 - 治安复杂区域、10 - 交通干线。当目录项为摄像机时可选。
    int PositionType = 0;
    // 摄像机安装位置室外、室内属性。1-室外、2-室内。当目录项为摄像机时可选, 缺省为1
    int RoomType = 1;
    // 摄像机用途属性。1-治安、2-交通、3-重点。当目录项为摄像机时可选。
    int UseType = 0;
    // 摄像机补光属性。1-无补光、2-红外补光、3-白光补光。当目录项为摄像机时可选, 缺省为1
    int SupplyLightType = 1;
    // 摄像机监视方位属性。1-东、2-西、3-南、4-北、5-东南、6-东北、7-西南、8-西北。当目录项为摄像机时且为固定摄像机或设置看守位摄像机时可选
    int DirectionType = 0;
    // 摄像机支持的分辨率,可有多个分辨率值,各个取值间以“/”分隔。分辨率取值参见附录 F中SDPf字段规定。当目录项为摄像机时可选
    std::string Resolution;
    // 虚拟组织所属的业务分组ID,业务分组根据特定的业务需求制定,一个业务分组包含一组特定的虚拟组织。
    deviceIDType BusinessGroupID;
    // 下载倍速范围(可选),各可选参数以“/”分隔,如设备支持1,2,4倍速下载则应写为“1/2/4”
    std::string DownloadSpeed;
    // 空域编码能力,取值0:不支持;1:1级增强(1个增强层);2:2级增强(2个增强层); 3:3级增强(3个增强层)(可选)
    int SVCSpaceSupportMode = 0;
    // 时域编码能力,取值0:不支持;1:1级增强;2:2级增强;3:3级增强(可选)
    int SVCTimeSupportMode = 0;
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
// 设备设置
const cmdType kDeviceConfig = "DeviceConfig";


} // namespace gb28181

#endif // _GB28181_MANSCDP_TYPE_H_
