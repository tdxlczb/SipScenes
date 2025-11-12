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
};