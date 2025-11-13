#pragma once
extern "C" {
//#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
}

#include <map>
#include "sip_define.h"
#include "sip_event.h"

class SipServer
{
public:
    SipServer();
    ~SipServer();
    // 加载Sip服务器以启动
    bool Init(const ServerInfo& serverInfo);
    // 事件处理循环
    void Loop();
    // 设置事件回调
    void SetSipEvent(SipEvent* pEvent);
    // 请求通话
    int Call(const std::string& callUid, const ClientInfo& clientInfo, const InviteOptions& options);
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
    int Request_INVITE(const ClientInfo& clientInfo, const InviteOptions& options);
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
private:
    const std::string kUserAgent = "GB28181-Server";
    const int kTimeout = 1800;
    const int kExpiry = 3600;

    ServerInfo m_serverInfo;        //服务器信息
    struct eXosip_t* m_pSipCtx;//Sip上下文
    bool m_isRun = false;//是否正在事件循环
    SipEvent* m_pSipEvent = nullptr;

    std::map<int, DialogInfo> m_mapCall;// <cid,callUid> exosip2管理的cid，绑定会话，用于发送Invite之后的预绑定
    std::map<std::string, DialogInfo> m_mapDialog;// <callUid,DialogInfo> 呼叫的自定义callId，绑定会话，用于成功创建会话之后的真实绑定
};

