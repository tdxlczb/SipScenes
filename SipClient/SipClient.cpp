#include "SipClient.h"

#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Dnsapi.lib")

extern "C" {
#include <eXosip2/eXosip.h>
}

#include "tools/http_digest.h"
#include "tools/log.h"

///////已含有功能：注册 注销 实时点播

SipClient::SipClient()
    : m_pSipCtx(nullptr)
    , m_isRun(false)
{
}

SipClient::~SipClient()
{
    m_mapClient.clear();
}

bool SipClient::Init(const ClientInfo& clientInfo)
{
    m_clientInfo = clientInfo;
    m_pSipCtx = eXosip_malloc();
    if (!m_pSipCtx) {
        LOGE("eXosip_malloc error");
        return false;
    }
    int ret = eXosip_init(m_pSipCtx);
    if (OSIP_SUCCESS != ret) {
        LOGE("eXosip_init error:%d", ret);
        return false;
    }
    /*
    i = eXosip_listen_addr(ctx, IPPROTO_TCP, NULL, port, AF_INET, 0);  // TCP
    i = eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, port, AF_INET, 0);  // UDP
    i = eXosip_listen_addr(ctx, IPPROTO_TCP, NULL, port, AF_INET, 1);  // TLS
    i = eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, port, AF_INET, 1);  // DTLS
    */
    ret = eXosip_listen_addr(m_pSipCtx, IPPROTO_UDP, m_clientInfo.sIp.c_str(), m_clientInfo.iPort, AF_INET, 0);
    if (OSIP_SUCCESS != ret) {
        LOGE("eXosip_listen_addr error:%d", ret);
        return false;
    }
    //eXosip_set_user_agent(m_pSipCtx, m_serverInfo.sUa.c_str());
    //if (eXosip_add_authentication_info(m_pSipCtx, m_serverInfo.sSipId.c_str(), m_serverInfo.sSipId.c_str()
    //    , m_serverInfo.sSipPass.c_str(), NULL, m_serverInfo.sSipRealm.c_str())) {
    //    LOGE("eXosip_add_authentication_info error");
    //    return false;
    //}
    LOGI("Client Listen");
    m_isRun = true;
    return true;
}

void SipClient::Loop()
{
    while (m_isRun) {
        eXosip_event_t* pSipEvt = eXosip_event_wait(m_pSipCtx, 0, 20);
        if (!pSipEvt) {
            //eXosip_automatic_action(m_pSipCtx);
            //osip_usleep(100000);// 100ms 的延时
            continue;
        }
        //eXosip_automatic_action(m_pSipCtx);
        this->SipEventHandle(pSipEvt);
        eXosip_event_free(pSipEvt);
    }
}

void SipClient::Register()
{
    Request_REGISTER();
}

void SipClient::Call(const std::string& dstUri)
{

}

void SipClient::SipEventHandle(eXosip_event_t* pSipEvt)
{
#define FIRST_SHOW
#ifdef FIRST_SHOW
    LOGI("type:%d", pSipEvt->type);
    this->DumpRequest(pSipEvt);
    this->DumpResponse(pSipEvt);
#endif // FIRST_SHOW

    switch (pSipEvt->type)
    {
    case EXOSIP_REGISTRATION_SUCCESS:    // 注册成功事件：SIP客户端成功注册到服务器
        // Method: REGISTER
        // Type: Response
        // Translate: 收到上级平台的 2xx 注册成功
        LOGI("==== REGISTER 200 OK ====");
        break;

    case EXOSIP_REGISTRATION_FAILURE:    // 注册失败事件：SIP客户端注册失败
        // Method: REGISTER
        // Type: Response
        // Translate: 收到上级平台的 3456xx 注册失败
        Request_REGISTER_Authorization(pSipEvt->response);
        break;

    case EXOSIP_CALL_INVITE:             // 收到INVITE请求：有新的呼叫请求
        // Method: INVITE
        // Type: Request
        // Translate: 收到上级平台发送的 INVITE 请求
#ifndef FIRST_SHOW
        this->DumpRequest(pSipEvt);
        this->DumpResponse(pSipEvt);
#endif // FIRST_SHOW
        //this->Response_INVITE(pSipEvt);
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
        osip_message_t* pMsg = nullptr;
        int iRet = eXosip_call_build_ack(m_pSipCtx, pSipEvt->did, &pMsg);
        if (!iRet && pMsg) {
            eXosip_call_send_ack(m_pSipCtx, pSipEvt->did, pMsg);
        }
        else {
            LOGE("eXosip_call_send_ack error=%d", iRet);
        }
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


void SipClient::DumpMessage(osip_message_t* pSipMsg)
{
    char* pMsg;
    size_t szLen;
    osip_message_to_str(pSipMsg, &pMsg, &szLen);
    if (pMsg) {
        LOGI("\nprint message start\n%s\nprint message end\n", pMsg);
    }
}

void SipClient::DumpRequest(eXosip_event_t* pSipEvt)
{
    char* pMsg;
    size_t szLen;
    osip_message_to_str(pSipEvt->request, &pMsg, &szLen);
    if (pMsg) {
        LOGI("\nprint request start\ntype=%d\n%s\nprint request end\n", pSipEvt->type, pMsg);
    }
    //osip_free(pMsg);
}

void SipClient::DumpResponse(eXosip_event_t* pSipEvt)
{
    char* pMsg;
    size_t szLen;
    osip_message_to_str(pSipEvt->response, &pMsg, &szLen);
    if (pMsg) {
        LOGI("\nprint response start\ntype=%d\n%s\nprint response end\n", pSipEvt->type, pMsg);
    }
    //osip_free(pMsg);
}

void SipClient::Request_REGISTER()
{
    char from[256], proxy[256], contact[256];
    snprintf(from, sizeof(from), "sip:%s@%s:%d", m_clientInfo.sUser.c_str(), m_clientInfo.sIp.c_str(), m_clientInfo.iPort);
    snprintf(proxy, sizeof(proxy), "sip:%s", m_clientInfo.sRegUri.c_str());
    //snprintf(contact, sizeof(contact), "sip:%s:%d", server_ip, server_port);

    eXosip_lock(m_pSipCtx);
    osip_message_t* pReg = NULL;
    int rid = eXosip_register_build_initial_register(m_pSipCtx, from, proxy, NULL, m_clientInfo.iSipExpiry, &pReg);
    if (rid < 0) {
        eXosip_unlock(m_pSipCtx);
        LOGI("eXosip_register_build_initial_register fail:%d", rid);
        return;
    }

    //// 设置扩展支持头部
    //osip_message_set_supported(pReg, "100rel");
    //osip_message_set_supported(pReg, "path");

    // 设置Allow值（RFC 3261 示例方法）
    const char* allow_val = "PRACK, INVITE, ACK, BYE, CANCEL, UPDATE, INFO, SUBSCRIBE, NOTIFY, REFER, MESSAGE, OPTIONS";
    //"INVITE, ACK, OPTIONS, CANCEL, BYE, UPDATE, INFO, MESSAGE, NOTIFY, PRACK, REFER";
    osip_message_set_allow(pReg, allow_val);

    int ret = eXosip_register_send_register(m_pSipCtx, rid, pReg);
    if (OSIP_SUCCESS != ret) {
        eXosip_unlock(m_pSipCtx);
        LOGI("eXosip_register_send_register fail:%d", ret);
        return;
    }
    eXosip_unlock(m_pSipCtx);
    LOGI(">>> initial REGISTER rid=%d", rid);
    m_rid = rid;
}

void SipClient::Request_REGISTER_Authorization(osip_message_t* pResponse)
{
    if (!pResponse || pResponse->status_code != 401)
        return;
    /* 二次 REGISTER：添加认证信息重新发送 */
    /* 取出 WWW-Authenticate 头 */
    osip_www_authenticate_t* www = NULL;
    int iRet = osip_message_get_www_authenticate(pResponse, 0, &www);
    if (OSIP_SUCCESS != iRet) {
        LOGI("osip_message_get_www_authenticate fail:%d", iRet);
        return;
    }
    if (www && www->realm && www->nonce) {
        /* 1. 重新构建 REGISTER（同 rid） */
        osip_message_t* pReg = NULL;
        iRet = eXosip_register_build_register(m_pSipCtx, m_rid, m_clientInfo.iSipExpiry, &pReg);
        if (OSIP_SUCCESS != iRet) {
            LOGI("eXosip_register_build_register fail:%d", iRet);
            return;
        }

        //注意字符串处理的方法，内部申请了内存，用完记得释放，避免内存泄露
        const char* method = "REGISTER";
        const char* username = m_clientInfo.sUser.c_str();
        char* realm = osip_strdup_without_quote(www->realm);
        char* nonce = osip_strdup_without_quote(www->nonce);
        const char* uri = std::string("sip:" + m_clientInfo.sRegUri).c_str();
        const char* algorithm = "MD5";
        const char* cnonce = "cnonce123456789";
        const char* message_qop = "auth";
        const char* nonce_count = "00000001";
        const char* password = m_clientInfo.sPwd.c_str();

        //提取请求中的交互信息并hash
        HASHHEX hashResponse = "";
        {
            HASHHEX hash1 = "", hash2 = "";
            DigestCalcHA1(algorithm, username, realm, password, nonce, nonce_count, hash1);
            DigestCalcResponse(hash1, nonce, nonce_count, cnonce, message_qop, 0, method, uri, hash2, hashResponse);
            LOGI("hashCalc hash1=%s response=%s", hash1, hashResponse);
        }

        osip_authorization_t* pHeader = nullptr;
        osip_authorization_init(&pHeader);//构建Authorization响应头
        osip_authorization_set_auth_type(pHeader, osip_strdup("Digest"));
        osip_authorization_set_username(pHeader, osip_enquote(username));
        osip_authorization_set_realm(pHeader, osip_enquote(realm));
        osip_authorization_set_nonce(pHeader, osip_enquote(nonce));
        osip_authorization_set_uri(pHeader, osip_enquote(uri));
        osip_authorization_set_response(pHeader, osip_enquote(hashResponse));
        osip_authorization_set_algorithm(pHeader, osip_strdup(algorithm));
        osip_authorization_set_cnonce(pHeader, osip_enquote(cnonce));
        osip_authorization_set_message_qop(pHeader, osip_strdup(message_qop));
        osip_authorization_set_nonce_count(pHeader, osip_strdup(nonce_count));

        osip_free(realm);
        osip_free(nonce);

        char* pDest = nullptr;
        osip_authorization_to_str(pHeader, &pDest);//将响应头的内容输出成字符串
        iRet = osip_message_set_authorization(pReg, pDest);
        if (OSIP_SUCCESS != iRet) {
            LOGI("osip_message_set_authorization fail:%d", iRet);
        }
        eXosip_lock(m_pSipCtx);
        iRet = eXosip_register_send_register(m_pSipCtx, m_rid, pReg);
        eXosip_unlock(m_pSipCtx);
        if (OSIP_SUCCESS != iRet) {
            LOGI("eXosip_register_send_register fail:%d", iRet);
        }
        osip_authorization_free(pHeader);
    }
}


void SipClient::Request_INVITE(const std::string& dstUri)
{
    char session_exp[1024] = { 0 };
    osip_message_t* msg = nullptr;
    char from[1024] = { 0 };
    char to[1024] = { 0 };
    char contact[1024] = { 0 };
    char sdp[2048] = { 0 };
    char head[1024] = { 0 };


    sprintf(from, "sip:%s@%s:%d", m_clientInfo.sUser.c_str(), m_clientInfo.sIp.c_str(), m_clientInfo.iPort);
    sprintf(to, "sip:%s", dstUri);
    snprintf(sdp, 2048,
        "v=0\r\n"
        "o=%s 0 0 IN IP4 %s\r\n"
        "s=Play\r\n"
        "c=IN IP4 %s\r\n"
        "t=0 0\r\n"
        "m=video %d TCP/RTP/AVP 96 98 97\r\n"
        "a=recvonly\r\n"
        "a=rtpmap:96 PS/90000\r\n"
        "a=rtpmap:98 H264/90000\r\n"
        "a=rtpmap:97 MPEG4/90000\r\n"
        "a=setup:passive\r\n"
        "a=connection:new\r\n"
        "y=0100000001\r\n"
        "f=\r\n", m_clientInfo.sUser.c_str(), m_clientInfo.sIp.c_str(), m_clientInfo.sIp.c_str(), m_clientInfo);

    int ret = eXosip_call_build_initial_invite(mSipCtx, &msg, to, from, nullptr, nullptr);
    if (ret) {
        LOGE("eXosip_call_build_initial_invite error: %s %s ret:%d", from, to, ret);
        return -1;
    }

    
    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp) - 1, "%i;refresher=uac", mInfo->getTimeout());
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    int call_id = eXosip_call_send_initial_invite(mSipCtx, msg);

    if (call_id > 0) {
        LOGI("eXosip_call_send_initial_invite success: call_id=%d", call_id);
    }
    else {
        LOGE("eXosip_call_send_initial_invite error: call_id=%d", call_id);
    }
    return ret;
}