#ifndef _GB28181_DEVICE_H_
#define _GB28181_DEVICE_H_

#include <string>

namespace gb28181 {

struct Device
{
    // 数据库自增ID
    int id = 0;

    // 设备国标编号
    std::string deviceId;

    // 设备名称
    std::string name;

    // 生产厂商
    std::string manufacturer;

    // 型号
    std::string model;

    // 固件版本
    std::string firmware;

    // 传输协议:UDP/TCP
    std::string transport;

    // 数据流传输模式
    // UDP:udp传输
    // TCP-ACTIVE：tcp主动模式
    // TCP-PASSIVE：tcp被动模式
    std::string streamMode;

    // 设备ip
    std::string ip;

    // 设备端口
    int port = 0;

    // 设备地址
    std::string hostAddress;

    // 是否在线，true为在线，false为离线
    bool onLine = false;

    // 注册时间
    std::string registerTime;

    // 心跳时间
    std::string keepaliveTime;

    // 心跳间隔，单位s，默认为20
    int heartBeatInterval = 20;

    // 心跳超时次数，默认为3
    int heartBeatCount = 3;

    // 定位功能支持情况。取值:0-不支持;1-支持 GPS定位;2-支持北斗定位(可选,默认取值为0
    int positionCapability = 0;

    // 通道个数
    int channelCount = 0;

    // 注册有效期
    int expires = 3600;

    // 创建时间
    std::string createTime;

    // 更新时间
    std::string updateTime;

    // 设备使用的媒体id
    std::string mediaServerId;

    // 字符集, 支持 UTF-8 与 GB2312
    std::string charset;

    // 目录订阅周期，0为不订阅
    int subscribeCycleForCatalog = 0;

    // 移动设备位置订阅周期，0为不订阅
    int subscribeCycleForMobilePosition = 0;

    // 移动设备位置信息上报时间间隔,单位:秒,默认值5
    int mobilePositionSubmissionInterval = 5;

    // 报警订阅周期，0为不订阅
    int subscribeCycleForAlarm = 0;

    // 是否开启ssrc校验，默认关闭，开启可以防止串流
    bool ssrcCheck = false;

    // 地理坐标系， 目前支持 WGS84,GCJ02, 此字段保留，暂无用
    std::string geoCoordSys;

    // 密码
    std::string password;

    // 收流IP
    std::string sdpIp;

    // SIP交互IP（设备访问平台的IP）
    std::string localIp;

    // 是否作为消息通道
    bool asMessageChannel = false;

    // 设备注册的事务信息
    //SipTransactionInfo sipTransactionInfo;

    // 控制语音对讲流程，释放收到ACK后发流
    bool broadcastPushAfterAck = false;

    // 所属服务Id
    std::string serverId;
};

}

#endif // _GB28181_DEVICE_H_
