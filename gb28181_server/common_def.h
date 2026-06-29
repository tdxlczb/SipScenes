#pragma once
#include <string>

struct StreamInfo
{
    std::string streamId;
    std::string deviceId;
    std::string ip;
    int port = 0;
    std::string channelId;
    int streamNumber = 0;//0主码流
    int tcpMode = 0;//0: 不监听端口 1: 监听端口 2: 主动连接到服务端
    std::string startTime; //2025-11-08 00:00:00
    std::string endTime; //2025-11-08 23:59:59

    int controlType = 0;// 1暂停，2恢复，3跳转，4倍速
    int64_t seekTime; //表示从startTime的多少秒之后开始播放
    double speed = 1.0;
};

struct MessageInfo
{
    std::string deviceId;
    std::string ip;
    int port = 0;
    bool update = false;
};


enum PTZControlType
{
    PTZ_CTRL_HALT = 0,			// 停止
    PTZ_CTRL_RIGHT,		        // 右转
    PTZ_CTRL_RIGHTUP,		    // 右上
    PTZ_CTRL_UP,		        // 上转
    PTZ_CTRL_LEFTUP,		    // 左上
    PTZ_CTRL_LEFT,		        // 左转
    PTZ_CTRL_LEFTDOWN,		    // 左下
    PTZ_CTRL_DOWN,		        // 下转
    PTZ_CTRL_RIGHTDOWN,		    // 右下
    PTZ_CTRL_ZOOM,              // 镜头放大/缩小
    PTZ_CTRL_IRIS,              // 光圈放大/缩小
    PTZ_CTRL_FOCUS,             // 镜头聚焦/放焦
};

struct ControlInfo
{
    std::string deviceId;
    std::string ip;
    int port = 0;

    PTZControlType controlType;
    int controlValue = 0; // 0-100
};