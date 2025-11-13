#include "sip_server.h"

#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <iphlpapi.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Dnsapi.lib")
#include <Windows.h>

extern "C" {
#include <eXosip2/eXosip.h>
}

#include "json/json.h"
#include "tools/http_digest.h"
#include "tools/httplib.h"
#include "tools/log.h"

namespace {

/* 更新nonce */
static void make_nonce(char* nonce, int len)
{
    snprintf(nonce, len, "%ld", (long)time(NULL));
}

}

SipServer::SipServer()
	: m_pSipCtx(nullptr)
    , m_isRun(false)
{
}

SipServer::~SipServer()
{
    eXosip_quit(m_pSipCtx);
}

bool SipServer::Init(const ServerInfo& serverInfo)
{
	m_serverInfo = serverInfo;
    m_pSipCtx = eXosip_malloc();
    if (!m_pSipCtx) {
        LOGE("eXosip_malloc error");
        return false;
    }
    int iRet = eXosip_init(m_pSipCtx);
    if (OSIP_SUCCESS != iRet) {
        LOGE("eXosip_init failed:%d", iRet);
        return false;
    }
    /*
    i = eXosip_listen_addr(ctx, IPPROTO_TCP, NULL, port, AF_INET, 0);  // TCP
    i = eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, port, AF_INET, 0);  // UDP
    i = eXosip_listen_addr(ctx, IPPROTO_TCP, NULL, port, AF_INET, 1);  // TLS
    i = eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, port, AF_INET, 1);  // DTLS
    */
    iRet = eXosip_listen_addr(m_pSipCtx, IPPROTO_UDP, nullptr, m_serverInfo.iPort, AF_INET, 0);
    if (OSIP_SUCCESS != iRet) {
        LOGE("eXosip_listen_addr failed:%d", iRet);
        return false;
    }

    //// 配置项
    //int val = 0;
    //eXosip_set_option(m_pSipCtx, EXOSIP_OPT_SET_TLS_VERIFY_CERTIFICATE, (void*)&val);
    //int keep_alive = 17000;
    //eXosip_set_option(m_pSipCtx, EXOSIP_OPT_UDP_KEEP_ALIVE, (void*)&keep_alive);
    //int dns_capabilities = 2;
    //eXosip_set_option(m_pSipCtx, EXOSIP_OPT_DNS_CAPABILITIES, (void*)&dns_capabilities);
    //int use_rport = 1;
    //eXosip_set_option(m_pSipCtx, EXOSIP_OPT_USE_RPORT, (void*)&use_rport);

    eXosip_set_user_agent(m_pSipCtx, kUserAgent.c_str());
    iRet = eXosip_add_authentication_info(m_pSipCtx, m_serverInfo.sUser.c_str(), m_serverInfo.sUser.c_str()
        , m_serverInfo.sPwd.c_str(), NULL, m_serverInfo.sRealm.c_str());
    if (OSIP_SUCCESS != iRet) {
        LOGE("eXosip_listen_addr failed:%d", iRet);
        return false;
    }
    LOGI("Server Listen");
    m_isRun = true;
	return true;
}

void SipServer::Loop()
{
    while (m_isRun) {
        eXosip_event_t* pSipEvt = eXosip_event_wait(m_pSipCtx, 0, 20);
        eXosip_lock(m_pSipCtx);
        eXosip_automatic_action(m_pSipCtx); // 处理一些自动行为
        eXosip_unlock(m_pSipCtx);

        if (!pSipEvt) {
            osip_usleep(100000);// 100ms 的延时
            continue;
        }
        this->EventHandle(pSipEvt);
        //释放事件资源
        eXosip_event_free(pSipEvt);
    }
}

void SipServer::SetSipEvent(SipEvent* pEvent)
{
    m_pSipEvent = pEvent;
}

int SipServer::Call(const std::string& callUid, const ClientInfo& clientInfo, const InviteOptions& options)
{
    auto iter = m_mapDialog.find(callUid);
    if (iter != m_mapDialog.end()) {
        Request_BYE(iter->second.exCallId, iter->second.exDialogId);
        m_mapDialog.erase(callUid);
    }

    int iRet = Request_INVITE(clientInfo, options);
    if (iRet > 0) {
        DialogInfo dialogInfo;
        dialogInfo.callUid = callUid;
        dialogInfo.exCallId = iRet;
        dialogInfo.clientInfo = clientInfo;
        m_mapCall[iRet] = dialogInfo;
        return 0;
    }
    return -1;
}

int SipServer::Hangup(const std::string& callUid, const ClientInfo& clientInfo)
{
    //Request_BYE(clientInfo);
    auto iter = m_mapDialog.find(callUid);
    if (iter != m_mapDialog.end()) {
        Request_BYE(iter->second.exCallId, iter->second.exDialogId);
    }
    return 0;
}

int SipServer::RequestMessage(const ClientInfo& clientInfo, const std::string& message)
{
    return Request_MESSAGE(clientInfo, message);
}

int SipServer::RequestNotify()
{
    return 0;
}

int SipServer::RequestInfo(const std::string& callUid, const std::string& body)
{
    auto iter = m_mapDialog.find(callUid);
    if (iter != m_mapDialog.end()) {
        Request_INFO(iter->second, body);
    }
    return 0;
}

void SipServer::EventHandle(eXosip_event_t* pSipEvt)
{
    //LOGI("received type:%d", pSipEvt->type);
    this->DumpRequest(pSipEvt);
    this->DumpResponse(pSipEvt);

    switch (pSipEvt->type)
    {
    case EXOSIP_REGISTRATION_SUCCESS:    // 注册成功事件：SIP客户端成功注册到服务器
        // Method: REGISTER
        // Type: Response
        // Translate: 收到上级平台的 2xx 注册成功
        break;

    case EXOSIP_REGISTRATION_FAILURE:    // 注册失败事件：SIP客户端注册失败
        // Method: REGISTER
        // Type: Response
        // Translate: 收到上级平台的 3456xx 注册失败
        break;

    case EXOSIP_CALL_INVITE:             // 收到INVITE请求：有新的呼叫请求
        // Method: INVITE
        // Type: Request
        // Translate: 收到上级平台发送的 INVITE 请求
        this->Response_INVITE(pSipEvt);
        break;

    case EXOSIP_CALL_REINVITE:           // 收到RE-INVITE请求：对现有呼叫的重新邀请
        // Method: INVITE
        // Type: Request
        // Translate: GB28181 无多方通话，所以无此情况
        break;

    case EXOSIP_CALL_NOANSWER:           // 呼叫未被响应：对方未接听
        // Method: NONE(INVITE)
        // Type: Event
        // Translate: 向下级平台发送的 INVITE 请求无响应
        break;

    case EXOSIP_CALL_PROCEEDING:         // 呼叫正在进行：对方已响应但尚未接听
        // Method: INVITE
        // Type: Response
        // Translate: 向下级平台发送的 INVITE 请求 1xx 响应
        break;

    case EXOSIP_CALL_RINGING:            // 呼叫正在振铃：对方已响应并正在振铃
        // Method: INVITE
        // Type: Response
        // Translate: 向下级平台发送的 INVITE 请求 1xx 响应
        break;

    case EXOSIP_CALL_ANSWERED: {           // 呼叫已接听：对方已响应并接听
        // Method: INVITE
        // Type: Response
        // Translate: 向下级平台发送的 INVITE 请求 2xx 响应
        this->Response_INVITE_ACK(pSipEvt);
        break;
    }
    case EXOSIP_CALL_REDIRECTED:         // 呼叫被重定向：呼叫被转发到其他地址
        // Method: INVITE
        // Type: Response
        // Translate: 向下级平台发送的 INVITE 请求 3xx 响应
        break;

    case EXOSIP_CALL_REQUESTFAILURE:     // 呼叫请求失败：可能是由于网络问题或其他原因
        // Method: INVITE
        // Type: Response
        // Translate: 向下级平台发送的 INVITE 请求 4xx 响应
        break;

    case EXOSIP_CALL_SERVERFAILURE:      // 呼叫失败：服务器端出现问题
        // Method: INVITE
        // Type: Response
        // Translate: 向下级平台发送的 INVITE 请求 5xx 响应
        break;

    case EXOSIP_CALL_GLOBALFAILURE:      // 呼叫失败：全局性问题（如不可达）
        // Method: INVITE
        // Type: Response
        // Translate: 向下级平台发送的 INVITE 请求 6xx 响应
        break;

    case EXOSIP_CALL_ACK:                // 收到ACK确认：对方已接听并确认
        // Method: ACK
        // Type: Request
        // Translate: 收到下级平台发送的 ACK 请求
        break;

    case EXOSIP_CALL_CANCELLED:          // 呼叫被取消：可能是由主叫方发起
        // Method: NONE
        // Type: Event
        // Translate: GB28181 无多方通话，所以无此情况
        break;

    case EXOSIP_CALL_MESSAGE_NEW:        // 收到新的消息
        // Method: MESSAGE, BYE, ...
        // Type: Request
        // Translate: 收到上级发送的 MESSAGE 消息(会话中)
        break;

    case EXOSIP_CALL_MESSAGE_PROCEEDING: // 消息正在处理中
        // Method: MESSAGE
        // Type: Response
        // Translate: 向下级平台发送的 MESSAGE 请求(会话中) 1xx 响应
        break;

    case EXOSIP_CALL_MESSAGE_ANSWERED:   // 消息已被响应
        // Method: MESSAGE
        // Type: Response
        // Translate: 向下级平台发送的 MESSAGE 请求(会话中) 2xx 响应
        if (MSG_IS_BYE(pSipEvt->request)) {
            this->Response_INVITE_BYE(pSipEvt);
        }
        break;

    case EXOSIP_CALL_MESSAGE_REDIRECTED: // 消息被重定向
        // Method: MESSAGE
        // Type: Response
        // Translate: 向下级平台发送的 MESSAGE 请求(会话中) 3xx 响应
        break;

    case EXOSIP_CALL_MESSAGE_REQUESTFAILURE: // 消息请求失败
        // Method: MESSAGE
        // Type: Response
        // Translate: 向下级平台发送的 MESSAGE 请求(会话中) 4xx 响应
        break;

    case EXOSIP_CALL_MESSAGE_SERVERFAILURE: // 消息失败：服务器端问题
        // Method: MESSAGE
        // Type: Response
        // Translate: 向下级平台发送的 MESSAGE 请求(会话中) 5xx 响应
        break;

    case EXOSIP_CALL_MESSAGE_GLOBALFAILURE: // 消息失败：全局性问题
        // Method: MESSAGE
        // Type: Response
        // Translate: 向下级平台发送的 MESSAGE 请求(会话中) 6xx 响应
        break;

    case EXOSIP_CALL_CLOSED:             // 呼叫已关闭：可能是因为正常结束或超时
        // Method: BYE
        // Type: Request
        // Translate: 收到上级或者下级发送的 BYE 请求
        break;

    case EXOSIP_CALL_RELEASED:           // 呼叫已释放：资源已清理
        // Method: NONE
        // Type: Event
        // Translate: 会话释放
        break;

    case EXOSIP_MESSAGE_NEW:             // 收到新的SIP消息请求
        // Method: MESSAGE, REGISTER, NOTIFY
        // Type: Request
        // Translate: 收到上级或者下级发送的 MESSAGE 消息
        if (MSG_IS_REGISTER(pSipEvt->request)) {
            this->Response_REGISTER(pSipEvt);
        }
        else if (MSG_IS_MESSAGE(pSipEvt->request)) {
            this->Response_MESSAGE(pSipEvt);
        }
//#define MSG_IS_BYE(msg) (MSG_IS_REQUEST(msg) && 0 == strcmp((msg)->sip_method, "BYE"))
        else if (strncmp(pSipEvt->request->sip_method, "BYE", 3) != 0) {
            LOGE("unknown1");
        }
        else {
            LOGE("unknown2");
        }
        break;

    case EXOSIP_MESSAGE_PROCEEDING:      // 消息正在处理中，已发送响应但尚未完成
        // Method: MESSAGE
        // Type: Response
        // Translate: 向上级或者下级发送的 MESSAGE 请求 1xx 响应
        break;

    case EXOSIP_MESSAGE_ANSWERED:        // 消息已被成功响应（例如，对方已回复）
        // Method: MESSAGE
        // Type: Response
        // Translate: 向上级或者下级发送的 MESSAGE 请求 2xx 响应
        break;

    case EXOSIP_MESSAGE_REDIRECTED:      // 消息被重定向到其他地址或目标
        // Method: MESSAGE
        // Type: Response
        // Translate: 向上级或者下级发送的 MESSAGE 请求 3xx 响应
        break;

    case EXOSIP_MESSAGE_REQUESTFAILURE:  // 消息请求失败，可能是由于客户端错误（如4xx响应）
        // Method: MESSAGE
        // Type: Response
        // Translate: 向上级或者下级发送的 MESSAGE 请求 4xx 响应
        break;

    case EXOSIP_MESSAGE_SERVERFAILURE:   // 消息处理失败，可能是由于服务器错误（如5xx响应）
        // Method: MESSAGE
        // Type: Response
        // Translate: 向上级或者下级发送的 MESSAGE 请求 5xx 响应
        break;

    case EXOSIP_MESSAGE_GLOBALFAILURE:   // 消息失败，全局性问题（如网络不可达，6xx响应）
        // Method: MESSAGE
        // Type: Response
        // Translate: 向上级或者下级发送的 MESSAGE 请求 6xx 响应
        break;

    case EXOSIP_SUBSCRIPTION_NOANSWER:   // 订阅请求未被响应
        // Method: NONE(SUBSCRIPTION)
        // Type: Event
        // Translate: 向下级平台发送的 SUBSCRIBE/REFER 请求无响应
        break;

    case EXOSIP_SUBSCRIPTION_PROCEEDING: // 订阅请求正在处理中
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: 向下级平台发送的 SUBSCRIBE/REFER 请求 1xx 响应
        break;

    case EXOSIP_SUBSCRIPTION_ANSWERED:   // 订阅请求已被响应
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: 向下级平台发送的 SUBSCRIBE/REFER 请求 2xx 响应
        break;

    case EXOSIP_SUBSCRIPTION_REDIRECTED: // 订阅请求被重定向
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: 向下级平台发送的 SUBSCRIBE/REFER 请求 3xx 响应
        break;

    case EXOSIP_SUBSCRIPTION_REQUESTFAILURE: // 订阅请求失败
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: 向下级平台发送的 SUBSCRIBE/REFER 请求 4xx 响应
        break;

    case EXOSIP_SUBSCRIPTION_SERVERFAILURE: // 订阅失败：服务器端问题
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: 向下级平台发送的 SUBSCRIBE/REFER 请求 5xx 响应
        break;

    case EXOSIP_SUBSCRIPTION_GLOBALFAILURE: // 订阅失败：全局性问题
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: 向下级平台发送的 SUBSCRIBE/REFER 请求 6xx 响应
        break;

    case EXOSIP_SUBSCRIPTION_NOTIFY:     // 收到通知：订阅内容有更新
        // Method: NOTIFY
        // Type: Request
        // Translate: 收到下级平台发送的 NOTIFY 请求
        break;

    case EXOSIP_IN_SUBSCRIPTION_NEW:     // 收到新的订阅请求
        // Method: SUBSCRIBE
        // Type: Request
        // Translate: 收到上级平台发送的 SUBSCRIBE/REFER 请求
        break;

    case EXOSIP_NOTIFICATION_NOANSWER:   // 通知未被响应
        // Method: NONE(NOTIFY)
        // Type: Event
        // Translate: 向上级平台发送的 Notify 请求无响应
        break;

    case EXOSIP_NOTIFICATION_PROCEEDING: // 通知正在处理中
        // Method: NOTIFY
        // Type: Response
        // Translate: 向上级平台发送的 Notify 请求 1xx 响应
        break;

    case EXOSIP_NOTIFICATION_ANSWERED:   // 通知已被响应
        // Method: NOTIFY
        // Type: Response
        // Translate: 向上级平台发送的 Notify 请求 2xx 响应
        break;

    case EXOSIP_NOTIFICATION_REDIRECTED: // 通知被重定向
        // Method: NOTIFY
        // Type: Response
        // Translate: 向上级平台发送的 Notify 请求 3xx 响应
        break;

    case EXOSIP_NOTIFICATION_REQUESTFAILURE: // 通知请求失败
        // Method: NOTIFY
        // Type: Response
        // Translate: 向上级平台发送的 Notify 请求 4xx 响应
        break;

    case EXOSIP_NOTIFICATION_SERVERFAILURE: // 通知失败：服务器端问题
        // Method: NOTIFY
        // Type: Response
        // Translate: 向上级平台发送的 Notify 请求 5xx 响应
        break;

    case EXOSIP_NOTIFICATION_GLOBALFAILURE: // 通知失败：全局性问题
        // Method: NOTIFY
        // Type: Response
        // Translate: 向上级平台发送的 Notify 请求 6xx 响应
        break;

    case EXOSIP_EVENT_COUNT:             // 事件总数：通常用于内部统计
        // Method: NONE
        // Type: None
        // Translate: 事件最大值
        break;

    default:                             // 默认分支：处理未知事件类型
        break;
    }
}

void SipServer::DumpMessage(osip_message_t* pSipMsg)
{
    char* pMsg;
    size_t szLen;
    osip_message_to_str(pSipMsg, &pMsg, &szLen);
    if (pMsg) {
        LOGI("\n==========print message start\n%s\n==========print message end\n", pMsg);
    }
    osip_free(pMsg);
}

void SipServer::DumpRequest(eXosip_event_t* pSipEvt)
{
    char* pMsg;
    size_t szLen;
    osip_message_to_str(pSipEvt->request, &pMsg, &szLen);
    if (pMsg) {
        LOGI("\n==========print request start\ntype=%d\n%s\n==========print request end\n", pSipEvt->type, pMsg);
    }
    osip_free(pMsg);
}

void SipServer::DumpResponse(eXosip_event_t* pSipEvt)
{
    char* pMsg;
    size_t szLen;
    osip_message_to_str(pSipEvt->response, &pMsg, &szLen);
    if (pMsg) {
        LOGI("\n==========print response start\ntype=%d\n%s\n==========print response end\n", pSipEvt->type, pMsg);
    }
    osip_free(pMsg);
}

void SipServer::Response_REGISTER(eXosip_event_t* pSipEvt)
{
    osip_authorization_t* pAuth = nullptr;
    osip_message_get_authorization(pSipEvt->request, 0, &pAuth);

    //如果缺乏来源Authorization信息则进入注册/注销信息响应构建
    if (nullptr == pAuth || nullptr == pAuth->username) {
        Response_REGISTER_401unauthorized(pSipEvt);
        return;
    }

    //注意字符串处理的方法，内部申请了内存，用完记得释放，避免内存泄露
    char* method = pSipEvt->request->sip_method; // REGISTER
    //提取字符串并消除其中的双引号
#define SIP_strdup(FIELD) char* FIELD = NULL; if (pAuth->FIELD) (FIELD) = osip_strdup_without_quote(pAuth->FIELD)
    SIP_strdup(algorithm); // MD5
    SIP_strdup(username); // SIP用户名
    SIP_strdup(realm); // sip服务器传给客户端，客户端携带并提交上来的sip服务域
    SIP_strdup(nonce); // sip服务器传给客户端，客户端携带并提交上来的nonce
    SIP_strdup(nonce_count);
    SIP_strdup(uri);
    SIP_strdup(response); // 客户端计算生成的认证字符串
    SIP_strdup(cnonce); // 客户端生成的随机数
    SIP_strdup(message_qop);
#undef SIP_strdup

    //提取请求中的交互信息并hash
    HASHHEX hashResponse = "";
    {
        const char* password = m_serverInfo.sPwd.c_str();
        //这里需要使用客户端的账号密码认证，所以一般情况是需要在sip协议以外在服务端注册登录的账号密码信息
        HASHHEX hash1 = "", hash2 = "";
        DigestCalcHA1(algorithm, username, realm, password, nonce, nonce_count, hash1);
        DigestCalcResponse(hash1, nonce, nonce_count, cnonce, message_qop, 0, method, uri, hash2, hashResponse);
        LOGI("hashCalc hash1=%s response=%s", hash1, hashResponse);
    }

    //提取主机端口用户名信息
    osip_contact_t* pContact = nullptr;
    osip_message_get_contact(pSipEvt->request, 0, &pContact);

    ClientInfo clientInfo;
    clientInfo.sUser = username;
    clientInfo.sIp = pContact->url->host;
    clientInfo.iPort = atoi(pContact->url->port);

    //hash验证，验证交互信息一致性
    if (response && 0 == memcmp(hashResponse, response, HASHHEXLEN)) {//一致则注册/注销此用户
        int iExpires = -1;
        osip_header_t* stExpires = nullptr;
        osip_message_get_expires(pSipEvt->request, 0, &stExpires);  
        LOGI("Expires:%s,%s", stExpires->hname, stExpires->hvalue);
        if (0 == atoi(stExpires->hvalue)) {
            //注销
            LOGI("unregister success,ip=%s,port=%d,device=%s", pContact->url->host, atoi(pContact->url->port), _strdup(username));
            this->MessageSendAnswer(pSipEvt, 200);//通知摄像头注销通过
        }
        else {
            LOGI("register success,ip=%s,port=%d,device=%s", pContact->url->host, atoi(pContact->url->port), _strdup(username));
            this->MessageSendAnswer(pSipEvt, 200);//通知摄像头注册通过
            if (m_pSipEvent)
                m_pSipEvent->OnRegister(clientInfo);
        }
    }
    else {//否则不予加入
        LOGI("register error, ip=%s,port=%d,device=%s", pContact->url->host, atoi(pContact->url->port), _strdup(username));
        this->MessageSendAnswer(pSipEvt, 401);//注册/注销失败
    }

    osip_free(algorithm);
    osip_free(username);
    osip_free(realm);
    osip_free(nonce);
    osip_free(nonce_count);
    osip_free(uri);
    osip_free(response);
    osip_free(cnonce);
    osip_free(message_qop);
}

void SipServer::Response_REGISTER_401unauthorized(eXosip_event_t* pSipEvt)
{
    //这里需要构建一个如下的消息体
    //WWW-Authenticate: Digest realm="3402000000", nonce="1762863282", algorithm=MD5, qop="auth" 

    char nonce[64] = {0};
    make_nonce(nonce, sizeof(nonce));

    char* pDest = nullptr;
    osip_www_authenticate_t* pHeader = nullptr;
    osip_www_authenticate_init(&pHeader);//构建WWW-Authenticate响应头
    osip_www_authenticate_set_auth_type(pHeader, osip_strdup("Digest"));//设置认证类型
    osip_www_authenticate_set_realm(pHeader, osip_enquote(m_serverInfo.sRealm.c_str()));//提供认证用的SIP服务器域
    osip_www_authenticate_set_nonce(pHeader, osip_enquote(nonce));//提供认证用的SIP服务随机数值
    osip_www_authenticate_set_algorithm(pHeader, osip_strdup("MD5"));
    osip_www_authenticate_set_qop_options(pHeader, osip_enquote("auth"));//这里要加双引号
    osip_www_authenticate_to_str(pHeader, &pDest);//将响应头的内容输出成字符串
    osip_message_t* pMsg = nullptr;
    int iRet = eXosip_message_build_answer(m_pSipCtx, pSipEvt->tid, 401, &pMsg);//构建一个响应
    if (OSIP_SUCCESS != iRet || !pMsg) {
        LOGE("eXosip_message_build_answer failed:%d", iRet);
    }

    iRet = osip_message_set_www_authenticate(pMsg, pDest);//将响应头字符串添加到响应信息中
    if (OSIP_SUCCESS != iRet) {
        LOGE("osip_message_set_www_authenticate failed:%d", iRet);
    }

    iRet = osip_message_set_content_type(pMsg, "Application/MANSCDP+xml");//设置响应消息的内容类型
    if (OSIP_SUCCESS != iRet) {
        LOGE("osip_message_set_content_type failed:%d", iRet);
    }

    //this->DumpMessage(pMsg);

    eXosip_lock(m_pSipCtx);
    iRet = eXosip_message_send_answer(m_pSipCtx, pSipEvt->tid, 401, pMsg);//发送响应
    eXosip_unlock(m_pSipCtx);
    if (OSIP_SUCCESS != iRet) {
        LOGE("eXosip_message_send_answer failed:%d", iRet);
    }
    LOGI("response_register_401unauthorized success");

    //回收资源
    osip_www_authenticate_free(pHeader);
    osip_free(pDest);
}

void SipServer::Response_MESSAGE(eXosip_event_t* pSipEvt)
{
    this->MessageSendAnswer(pSipEvt, 200);
    //提取主机端口用户名信息
    osip_contact_t* pContact = nullptr;
    osip_message_get_contact(pSipEvt->request, 0, &pContact);

    ClientInfo clientInfo;
    if (pContact) {
        clientInfo.sUser = pContact->url->username;
        clientInfo.sIp = pContact->url->host;
        clientInfo.iPort = atoi(pContact->url->port);
    }
    osip_body_t* pBody = nullptr;
    osip_message_get_body(pSipEvt->request, 0, &pBody);
    if (m_pSipEvent)
        m_pSipEvent->OnMessage(clientInfo, std::string(pBody->body));
}

void SipServer::Response_INVITE(eXosip_event_t* pSipEvt)
{
    char sSessionExp[1024] = { 0 };
    osip_message_t* pMsg = nullptr;
    char sFrom[1024] = { 0 };
    char sTo[1024] = { 0 };
    char sContact[1024] = { 0 };
    char sSdp[2048] = { 0 };
    char sHead[1024] = { 0 };
    std::string srcPort;
    std::cin >> srcPort;
    /*PCMA
    v=0
    o=34020000001320000022 2798 2798 IN IP4 172.16.19.236
    s=Play
    c=IN IP4 172.16.19.236
    t=0 0
    m=audio 15062 RTP/AVP 8 96
    a=recvonly
    a=rtpmap:8 PCMA/8000
    a=rtpmap:96 PS/90000
    y=0200000017
    f=v/////a/1/8/1
    */
    sprintf_s(sSdp, 2048,
        "v=0\r\n"
        "o=%s 0 0 IN IP4 %s\r\n"
        "s=Play\r\n"
        "c=IN IP4 %s\r\n"
        "t=0 0\r\n"
        "m=audio %d RTP/AVP 8 96\r\n"// 104 96
        "a=sendonly\r\n"
        "a=rtpmap:8 PCMA/8000\r\n"//a=rtpmap:104 mpeg4-generic/16000
        "a=rtpmap:96 PS/90000\r\n"
        "y=0200000017\r\n"//a/-1/6/3
        "f=v/////a/1/8/1\r\n", m_serverInfo.sUser.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.sIp.c_str(), /*m_serverInfo.iRtpPort*/atoi(srcPort.c_str()));

    int iRet = eXosip_call_build_answer(m_pSipCtx, pSipEvt->tid, 200, &pMsg);
    if (iRet) {
        LOGE("eXosip_call_build_answer error: %s %s ret:%d", sFrom, sTo, iRet);
        return;
    }
    
    osip_message_set_body(pMsg, sSdp, strlen(sSdp));
    osip_message_set_content_type(pMsg, "application/sdp");

    int iCallId = eXosip_call_send_answer(m_pSipCtx, pSipEvt->tid, 200, pMsg);

    if (iCallId > 0) {
        LOGI("eXosip_call_send_answer success: iCallId=%d", iCallId);
    }
    else {
        LOGE("eXosip_call_send_answer error: iCallId=%d", iCallId);
    }
    
}

void SipServer::Response_INVITE_ACK(eXosip_event_t* pSipEvt)
{
    //此时收到INVITE的200回复说明会话建立成功
    auto iter = m_mapCall.find(pSipEvt->cid);
    if (iter == m_mapCall.end())
        return;

    osip_message_t* pMsg = nullptr;
    int ret = eXosip_call_build_ack(m_pSipCtx, pSipEvt->did, &pMsg);
    if (!ret && pMsg) {
        ret = eXosip_call_send_ack(m_pSipCtx, pSipEvt->did, pMsg);
        if (ret >= 0) {
            DialogInfo dlg = iter->second;
            dlg.exDialogId = pSipEvt->did;
            m_mapDialog.emplace(dlg.callUid, dlg);
        }
    }
    else {
        LOGE("eXosip_call_send_ack error=%d", ret);
    }
}

void SipServer::Response_INVITE_BYE(eXosip_event_t* pSipEvt)
{
    auto iter = m_mapCall.find(pSipEvt->cid);
    if (iter == m_mapCall.end())
        return;
    
    m_mapDialog.erase(iter->second.callUid);
}

void SipServer::MessageSendAnswer(eXosip_event_t* pSipEvt, int iStatus)
{
    int iRet = 0;
    osip_message_t* pMsg = nullptr;
    iRet = eXosip_message_build_answer(m_pSipCtx, pSipEvt->tid, iStatus, &pMsg);
    if (iRet == 0 && pMsg != nullptr)
    {
        eXosip_lock(m_pSipCtx);
        eXosip_message_send_answer(m_pSipCtx, pSipEvt->tid, iStatus, pMsg);//发送响应
        eXosip_unlock(m_pSipCtx);
    }
    else {
        LOGE("MessageSendAnswer error iStatus=%d,iRet=%d,pMsg=%d", iStatus, iRet, pMsg != nullptr);
    }
    //osip_message_free(pMsg);//发送操作是将消息对象扔到线程队列，这里不能直接释放消息内存
}

int SipServer::Request_INVITE(const ClientInfo& clientInfo, const InviteOptions& options)
{
    char sFrom[1024] = { 0 };
    char sTo[1024] = { 0 };
    sprintf_s(sFrom, "sip:%s@%s", m_serverInfo.sUser.c_str(), m_serverInfo.sDomain.c_str());
    sprintf_s(sTo, "sip:%s@%s:%d", clientInfo.sUser.c_str(), clientInfo.sIp.c_str(), clientInfo.iPort);

    osip_message_t* pMsg = nullptr;
    int iRet = eXosip_call_build_initial_invite(m_pSipCtx, &pMsg, sTo, sFrom, nullptr, nullptr);
    if (iRet) {
        LOGE("eXosip_call_build_initial_invite error: %s %s ret:%d", sFrom, sTo, iRet);
        return -1;
    }

    osip_message_set_body(pMsg, options.sdp.c_str(), options.sdp.length());
    osip_message_set_content_type(pMsg, "application/sdp");
    osip_message_set_header(pMsg, "Allow", "INVITE, ACK, INFO, CANCEL, BYE, OPTIONS, REGISTER, MESSAGE");
    if (!options.subject.empty()) {
        osip_message_set_header(pMsg, "Subject", options.subject.c_str());
    }
    //char sSessionExp[1024] = { 0 };
    //snprintf(sSessionExp, sizeof(sSessionExp) - 1, "%i;refresher=uac", kTimeout);
    //osip_message_set_header(pMsg, "Session-Expires", sSessionExp);
    //osip_message_set_supported(pMsg, "timer"); //会话计时器

    int iCallId = eXosip_call_send_initial_invite(m_pSipCtx, pMsg);
    if (iCallId > 0) {
        LOGI("eXosip_call_send_initial_invite success: iCallId=%d", iCallId);
    }
    else {
        LOGE("eXosip_call_send_initial_invite error: iCallId=%d", iCallId);
    }
    return iCallId;
}

int SipServer::Request_BYE(int cid, int did)
{
    eXosip_lock(m_pSipCtx);
    int iRet = eXosip_call_terminate(m_pSipCtx, cid, did);
    eXosip_unlock(m_pSipCtx);
    return iRet;
}

int SipServer::Request_BYE(const ClientInfo& clientInfo)
{
    osip_message_t* pMsg = nullptr;
    char sFrom[1024] = { 0 };
    char sTo[1024] = { 0 };
    char sContact[1024] = { 0 };

    sprintf_s(sContact, "sip:%s@%s:%d", m_serverInfo.sUser.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.iPort);
    sprintf_s(sFrom, "sip:%s@%s", m_serverInfo.sUser.c_str(), m_serverInfo.sRealm.c_str());
    sprintf_s(sTo, "sip:%s@%s:%d", clientInfo.sUser.c_str(), clientInfo.sIp.c_str(), clientInfo.iPort);
    int iRet = eXosip_message_build_request(m_pSipCtx, &pMsg, "BYE", sTo, sFrom, nullptr);
    if (iRet) {
        LOGE("eXosip_call_build_initial_invite error: %s %s ret:%d", sFrom, sTo, iRet);
        return -1;
    }

    osip_message_set_contact(pMsg, sContact);
    
    char sBranch[1024] = "z9hG4bK4164472032";
    char sCallId[1024] = "3400557450";
    char sCSeq[1024] = "21";
    char sFromTag[1024] = "1868317067";
    char sToTag[1024] = "7865737";

    //osip_call_id_free(pMsg->call_id);
    //pMsg->call_id = NULL;
    //osip_call_id_t* cid = NULL;
    //osip_call_id_init(&cid);
    //osip_call_id_set_number(cid, osip_strdup(sCallId));
    //pMsg->call_id = cid;

    // 设置必要的头域
    //osip_message_set_call_id(pMsg, sCallId);
    //osip_message_set_cseq(pMsg, sCSeq);

    //直接调用设置需要释放原来的数据
    osip_free(pMsg->call_id->number);
    osip_free(pMsg->cseq->number);

    osip_call_id_set_number(pMsg->call_id, osip_strdup(sCallId));
    osip_cseq_set_number(pMsg->cseq, osip_strdup(sCSeq));

    osip_via_t* via = NULL;
    osip_message_get_via(pMsg, 0, &via);
    osip_uri_param_t* vp = nullptr;
    osip_uri_param_get_byname(&via->via_params, osip_strdup("branch"), &vp);
    vp->gvalue = osip_strdup(sBranch);

    // 设置From和To标签
    osip_from_t* from = osip_message_get_from(pMsg);
    osip_to_t* to = osip_message_get_to(pMsg);
    osip_uri_param_t* fp = nullptr;
    osip_uri_param_get_byname(&from->gen_params, osip_strdup("tag"), &fp);
    fp->gvalue = osip_strdup(sFromTag);
    //不使用osip_strdup会崩溃
    if (to) osip_to_set_tag(to, osip_strdup(sToTag));

    DumpMessage(pMsg);
    int iCallId = eXosip_message_send_request(m_pSipCtx, pMsg);
    if (iCallId > 0) {
        LOGI("eXosip_message_send_request success: iCallId=%d", iCallId);
    }
    else {
        LOGE("eXosip_message_send_request error: iCallId=%d", iCallId);
    }
    return iCallId;
}

int SipServer::Request_MESSAGE(const ClientInfo& clientInfo, const std::string& message)
{
    LOGI("MESSAGE");

    char sFrom[1024] = { 0 };
    char sTo[1024] = { 0 };
    sprintf_s(sFrom, "sip:%s@%s", m_serverInfo.sUser.c_str(), m_serverInfo.sDomain.c_str());
    sprintf_s(sTo, "sip:%s@%s:%d", clientInfo.sUser.c_str(), clientInfo.sIp.c_str(), clientInfo.iPort);

    osip_message_t* pMsg = nullptr;
    int iRet = eXosip_message_build_request(m_pSipCtx, &pMsg, "MESSAGE", sTo, sFrom, nullptr);
    if (iRet) {
        LOGE("eXosip_call_build_initial_invite error: %s %s ret:%d", sFrom, sTo, iRet);
        //osip_message_free(pMsg);//创建失败内部会释放内存，这里不需要释放
        return -1;
    }
    osip_message_set_body(pMsg, message.c_str(), message.length());
    osip_message_set_content_type(pMsg, "Application/MANSCDP+xml");//设置请求消息的内容类型

    int iCallId = eXosip_message_send_request(m_pSipCtx, pMsg);
    if (iCallId > 0) {
        LOGI("eXosip_message_send_request success: iCallId=%d", iCallId);
    }
    else {
        LOGE("eXosip_message_send_request error: iCallId=%d", iCallId);
    }
    //osip_message_free(pMsg);//发送操作是将消息对象扔到线程队列，这里不能直接释放消息内存
    return iCallId;
}

int SipServer::Request_NOTIFY(const ClientInfo& clientInfo, const std::string& message)
{
    return 0;
}

int SipServer::Request_INFO(const DialogInfo& dlgInfo, const std::string& body)
{
    osip_message_t* info = nullptr;
    int ret = eXosip_call_build_info(m_pSipCtx, dlgInfo.exDialogId, &info);
    if (ret != 0) {
        LOGE("eXosip_call_build_info error: ret=%d", ret);
        //osip_message_free(info);//创建失败内部会释放内存，这里不需要释放
        return -1;
    }

    osip_message_set_content_type(info, "APPLICATION/MANSRTSP");
    osip_message_set_body(info, body.c_str(), body.length());
    ret = eXosip_call_send_request(m_pSipCtx, dlgInfo.exDialogId, info);
    LOGE("eXosip_call_send_request: ret=%d", ret);
    //osip_message_free(info);//发送操作是将消息对象扔到线程队列，这里不能直接释放消息内存
    return ret;
}
