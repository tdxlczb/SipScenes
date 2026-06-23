#pragma once

#include <iostream>
#include <string>
#include <map>

struct ServerInfo {
    std::string ip;//服务器IP
    int port = 0;//服务器端口
    std::string id; //服务器名称或Id
    std::string password;//服务器密码，如果设置了服务器密码，则要求所有注册的账号都使用该密码，否则需要账号管理功能
    std::string domain;//服务器域名，用于寻址和路由，拼接sip-url，为空时会使用ip:port
    std::string realm;//服务器域，用于认证和鉴权，为空时会使用domain
};

struct ClientInfo {
    std::string id;// 客户端名称
    std::string ip; // 客户端ip
    int port = 0; // 客户端端口
    bool isReg = false;//是否注册
};

//主要用于记录invite的会话信息
struct DialogInfo
{
    std::string callUid;//自定义uid，用于绑定
    int exCallId = 0;
    int exDialogId = 0;
    std::string callId;
    std::string branch;
    int cseqNum = 0; //会话最新的cseq
    std::string fromTag;
    std::string toTag;
    ClientInfo clientInfo;
};

//invite的信息
struct InviteOptions
{
    std::string subject;
    std::string sdp;
};