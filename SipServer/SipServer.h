#pragma once
extern "C" {
#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
}
#include <iostream>
#include <string>
#include <map>

struct ServerInfo {
    std::string sUa;//SIP服务器名称
    std::string sNonce;//SIP服务随机数值
    std::string sIp;//SIP服务IP
    int iPort = 0;//SIP服务端口
    std::string sSipId; //SIP服务器ID
    std::string sSipRealm;//SIP服务器域
    std::string sSipPass;//SIP password
    int iSipTimeout = 0; //SIP timeout
    int iSipExpiry = 0;// SIP默认到期时间:秒
    int iRtpPort = 0; //SIP-RTP服务端口
};

struct ClientInfo {
    std::string sIp; // 客户端ip
    int iPort = 0; // 客户端端口
    std::string sDevice;// 340200000013200000024
    bool isReg = false;//是否注册
    int iRtpPort = 0; //SIP-RTP客户端端口
};

class SipServer
{
public:
    SipServer();
    ~SipServer();
    /// <summary>
    /// 加载Sip服务器以启动
    /// </summary>
    /// <param name="serverInfo">Sip服务器信息</param>
    /// <returns>是否加载并启动成功</returns>
    bool Init(const ServerInfo& serverInfo);
    /// <summary>
    /// 事件处理循环
    /// </summary>
    void Loop();
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
    ServerInfo m_serverInfo;        //服务器信息
    struct eXosip_t* m_pSipCtx;//Sip上下文
    bool m_isRun = false;//是否正在事件循环
    std::map<std::string, ClientInfo> m_mapClient;// <DeviceID,SipClient> 客户端map
};

