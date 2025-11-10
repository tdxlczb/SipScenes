#pragma once
extern "C" {
//#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
}
#include <iostream>
#include <string>
#include <map>

struct ServerInfo {
    std::string sIp;//服务器IP
    int iPort = 0;//服务器端口
    std::string sUser; //服务器名称或Id
    std::string sPwd;//服务器密码，如果设置了服务器密码，则要求所有注册的账号都使用该密码，否则需要账号管理功能
    std::string sDomain;//服务器域名，用于寻址和路由，拼接sip-url，为空时会使用ip:port
    std::string sRealm;//服务器域，用于认证和鉴权，为空时会使用domain
};

struct ClientInfo {
    std::string sUser;// 340200000013200000024
    std::string sIp; // 客户端ip
    int iPort = 0; // 客户端端口
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

class SipServer
{
public:
    SipServer();
    ~SipServer();
    // 加载Sip服务器以启动
    bool Init(const ServerInfo& serverInfo);
    // 事件处理循环
    void Loop();
    // 请求通话
    int Call(const std::string& callUid, const ClientInfo& clientInfo, const std::string& sdp);
    // 停止通话
    int Hangup(const std::string& callUid, const ClientInfo& clientInfo);
    // 发送MESSAGE消息
    int RequestMessage(const ClientInfo& clientInfo, const std::string& message);
    // 发送NOTIFY消息
    int RequestNotify();
    // 发送INFO消息
    int RequestInfo(const std::string& callUid, const std::string& body);
public:
    /// <summary>
    /// Sip事件处理
    /// </summary>
    /// <param name="pSipEvt">Sip事件</param>
    void EventHandle(eXosip_event_t* pSipEvt);
    /// <summary>
    /// 格式化打印消息
    /// </summary>
    /// <param name="pSipMsg">Sip消息</param>
    void DumpMessage(osip_message_t* pSipMsg);
    void DumpRequest(eXosip_event_t* pSipEvt);
    void DumpResponse(eXosip_event_t* pSipEvt);
    /// <summary>
    /// 请求处理：sip_method = REGISTER
    /// </summary>
    /// <param name="pSipEvt">Sip事件</param>
    void Response_REGISTER(eXosip_event_t* pSipEvt);
    /// <summary>
    /// Response_REGISTER的子处理(未经授权)
    /// </summary>
    /// <param name="pSipEvt">Sip事件</param>
    void Response_REGISTER_401unauthorized(eXosip_event_t* pSipEvt);
    /// <summary>
    /// 请求处理：sip_method = MESSAGE
    /// </summary>
    /// <param name="pSipEvt">Sip事件</param>
    void Response_MESSAGE(eXosip_event_t* pSipEvt);
    /// <summary>
    /// 请求处理：sip_method = INVITE
    /// </summary>
    /// <param name="pSipEvt">Sip事件</param>
    void Response_INVITE(eXosip_event_t* pSipEvt);
    void Response_INVITE_ACK(eXosip_event_t* pSipEvt);
    void Response_INVITE_BYE(eXosip_event_t* pSipEvt);
    /// <summary>
    /// 发送响应
    /// </summary>
    /// <param name="pSipEvt">Sip事件</param>
    /// <param name="iStatus">响应码</param>
    void MessageSendAnswer(eXosip_event_t* pSipEvt, int iStatus);
    /// <summary>
    /// 发送请求：INVITE
    /// </summary>
    /// <param name="clientInfo">发送的目的对象</param>
    int Request_INVITE(const ClientInfo& clientInfo, const std::string& sdp);
    /// <summary>
    /// 发送请求：BYE
    /// </summary>
    /// <param name="clientInfo">发送的目的对象</param>
    int Request_BYE(int cid, int did);
    int Request_BYE(const ClientInfo& clientInfo);
    /// <summary>
    /// 发送请求：MESSAGE
    /// </summary>
    /// <param name="clientInfo">发送的目的对象</param>
    int Request_MESSAGE(const ClientInfo& clientInfo, const std::string& body);
    /// <summary>
    /// 发送请求：NOTIFY
    /// </summary>
    /// <param name="clientInfo">发送的目的对象</param>
    int Request_NOTIFY(const ClientInfo& clientInfo, const std::string& body);
    /// <summary>
    /// 发送请求：INFO
    /// </summary>
    /// <param name="DialogInfo">发送的目的对象</param>
    int Request_INFO(const DialogInfo& dlgInfo, const std::string& body);
    /// <summary>
    /// 解析xml，提取出其中一项的值
    /// </summary>
    /// <param name="pData"></param>
    /// <param name="pSMark"></param>
    /// <param name="isWithSMake"></param>
    /// <param name="pEMark"></param>
    /// <param name="isWithEMake"></param>
    /// <param name="pDest"></param>
    /// <returns></returns>
    bool parseXml(const char* pData, const char* pSMark, bool isWithSMake, const char* pEMark, bool isWithEMake, char* pDest);
private:
    const std::string kUserAgent = "GB28181-Server";
    const int kTimeout = 1800;
    const int kExpiry = 3600;

    ServerInfo m_serverInfo;        //服务器信息
    struct eXosip_t* m_pSipCtx;//Sip上下文
    bool m_isRun = false;//是否正在事件循环

    std::map<int, DialogInfo> m_mapCall;// <cid,callUid> exosip2管理的cid，绑定会话，用于发送Invite之后的预绑定
    std::map<std::string, DialogInfo> m_mapDialog;// <callUid,DialogInfo> 呼叫的自定义callId，绑定会话，用于成功创建会话之后的真实绑定
};

