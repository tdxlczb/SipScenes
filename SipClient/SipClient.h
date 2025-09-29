#pragma once
extern "C" {
#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
}
#include <iostream>
#include <string>
#include <map>

struct ClientInfo {
    std::string sUser;
    std::string sPwd;
    std::string sRealm; // 域
    std::string sIp; // 客户端ip
    int iPort = 0; // 客户端端口
    int iSipExpiry = 0; //SIP注册到期时间:秒

    std::string sRegUri;//注册的服务端地址，如 172.16.19.108:5060 或者 sip.pjsip.org
};


class SipClient
{
public:
    SipClient();
    ~SipClient();
    /// <summary>
    /// 初始化Sip客户端
    /// </summary>
    /// <param name="serverInfo">Sip客户端信息</param>
    /// <returns>是否初始化成功</returns>
    bool Init(const ClientInfo& clientInfo);
    /// <summary>
    /// 事件处理循环
    /// </summary>
    void Loop();

    /// <summary>
    /// 注册
    /// </summary>
    void Register();

private:
    /// <summary>
    /// Sip事件处理
    /// </summary>
    /// <param name="pSipEvt">Sip事件</param>
    void SipEventHandle(eXosip_event_t* pSipEvt);
    /// <summary>
    /// 格式化打印消息
    /// </summary>
    /// <param name="pSipMsg">Sip消息</param>
    void DumpMessage(osip_message_t* pSipMsg);
    /// <summary>
    /// 格式化打印请求
    /// </summary>
    /// <param name="pSipEvt">Sip事件</param>
    void DumpRequest(eXosip_event_t* pSipEvt);
    /// <summary>
    /// 格式化打印响应
    /// </summary>
    /// <param name="pSipEvt">Sip事件</param>
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
    /// <summary>
    /// 发送响应
    /// </summary>
    /// <param name="pSipEvt">Sip事件</param>
    /// <param name="iStatus">响应码</param>
    void MessageSendAnswer(eXosip_event_t* pSipEvt, int iStatus);
    /// <summary>
    /// 发送请求：REGISTER
    /// </summary>
    /// <param name="clientInfo">发送的目的对象</param>
    void Request_REGISTER();
    /// <summary>
    /// 发送请求：REGISTER 携带认证信息
    /// </summary>
    /// <param name="pResponse">Sip响应的消息</param>
    void Request_REGISTER_Authorization(osip_message_t* pResponse);
    /// <summary>
    /// 发送请求：INVITE
    /// </summary>
    /// <param name="clientInfo">发送的目的对象</param>
    void Request_INVITE(const ClientInfo& clientInfo);
    /// <summary>
    /// 发送请求：MESSAGE
    /// </summary>
    /// <param name="clientInfo">发送的目的对象</param>
    void Request_MESSAGE(const ClientInfo& clientInfo);
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
    ClientInfo m_clientInfo;   //客户端信息
    struct eXosip_t* m_pSipCtx;//Sip上下文
    bool m_isRun = false;//是否正在事件循环
    int m_rid = 0;//注册id
    std::map<std::string, ClientInfo> m_mapClient;// <DeviceID,SipClient> 客户端map
};

