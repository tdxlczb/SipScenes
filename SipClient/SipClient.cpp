#include "SipClient.h"
#include "Log.h"

#include <winsock2.h>

extern "C" {
#include "HTTPDigest.h"
}

///////�Ѻ��й��ܣ�ע�� ע�� ʵʱ�㲥

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
            //osip_usleep(100000);// 100ms ����ʱ
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
    case EXOSIP_REGISTRATION_SUCCESS:    // ע��ɹ��¼���SIP�ͻ��˳ɹ�ע�ᵽ������
        // Method: REGISTER
        // Type: Response
        // Translate: �յ��ϼ�ƽ̨�� 2xx ע��ɹ�
        LOGI("==== REGISTER 200 OK ====");
        break;

    case EXOSIP_REGISTRATION_FAILURE:    // ע��ʧ���¼���SIP�ͻ���ע��ʧ��
        // Method: REGISTER
        // Type: Response
        // Translate: �յ��ϼ�ƽ̨�� 3456xx ע��ʧ��
        Request_REGISTER_Authorization(pSipEvt->response);
        break;

    case EXOSIP_CALL_INVITE:             // �յ�INVITE�������µĺ�������
        // Method: INVITE
        // Type: Request
        // Translate: �յ��ϼ�ƽ̨���͵� INVITE ����
#ifndef FIRST_SHOW
        this->DumpRequest(pSipEvt);
        this->DumpResponse(pSipEvt);
#endif // FIRST_SHOW
        //this->Response_INVITE(pSipEvt);
        break;

    case EXOSIP_CALL_REINVITE:           // �յ�RE-INVITE���󣺶����к��е���������
        // Method: INVITE
        // Type: Request
        // Translate: GB28181 �޶෽ͨ���������޴����
        break;

    case EXOSIP_CALL_NOANSWER:           // ����δ����Ӧ���Է�δ����
        // Method: NONE(INVITE)
        // Type: Event
        // Translate: ���¼�ƽ̨���͵� INVITE ��������Ӧ
        break;

    case EXOSIP_CALL_PROCEEDING:         // �������ڽ��У��Է�����Ӧ����δ����
        // Method: INVITE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� INVITE ���� 1xx ��Ӧ
        break;

    case EXOSIP_CALL_RINGING:            // �����������壺�Է�����Ӧ����������
        // Method: INVITE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� INVITE ���� 1xx ��Ӧ
        break;

    case EXOSIP_CALL_ANSWERED: {           // �����ѽ������Է�����Ӧ������
        // Method: INVITE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� INVITE ���� 2xx ��Ӧ
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
    case EXOSIP_CALL_REDIRECTED:         // ���б��ض��򣺺��б�ת����������ַ
        // Method: INVITE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� INVITE ���� 3xx ��Ӧ
        break;

    case EXOSIP_CALL_REQUESTFAILURE:     // ��������ʧ�ܣ������������������������ԭ��
        // Method: INVITE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� INVITE ���� 4xx ��Ӧ
        break;

    case EXOSIP_CALL_SERVERFAILURE:      // ����ʧ�ܣ��������˳�������
        // Method: INVITE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� INVITE ���� 5xx ��Ӧ
        break;

    case EXOSIP_CALL_GLOBALFAILURE:      // ����ʧ�ܣ�ȫ�������⣨�粻�ɴ
        // Method: INVITE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� INVITE ���� 6xx ��Ӧ
        break;

    case EXOSIP_CALL_ACK:                // �յ�ACKȷ�ϣ��Է��ѽ�����ȷ��
        // Method: ACK
        // Type: Request
        // Translate: �յ��¼�ƽ̨���͵� ACK ����
        break;

    case EXOSIP_CALL_CANCELLED:          // ���б�ȡ���������������з�����
        // Method: NONE
        // Type: Event
        // Translate: GB28181 �޶෽ͨ���������޴����
        break;

    case EXOSIP_CALL_MESSAGE_NEW:        // �յ��µ���Ϣ
        // Method: MESSAGE, BYE, ...
        // Type: Request
        // Translate: �յ��ϼ����͵� MESSAGE ��Ϣ(�Ự��)
        break;

    case EXOSIP_CALL_MESSAGE_PROCEEDING: // ��Ϣ���ڴ�����
        // Method: MESSAGE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� MESSAGE ����(�Ự��) 1xx ��Ӧ
        break;

    case EXOSIP_CALL_MESSAGE_ANSWERED:   // ��Ϣ�ѱ���Ӧ
        // Method: MESSAGE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� MESSAGE ����(�Ự��) 2xx ��Ӧ
        break;

    case EXOSIP_CALL_MESSAGE_REDIRECTED: // ��Ϣ���ض���
        // Method: MESSAGE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� MESSAGE ����(�Ự��) 3xx ��Ӧ
        break;

    case EXOSIP_CALL_MESSAGE_REQUESTFAILURE: // ��Ϣ����ʧ��
        // Method: MESSAGE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� MESSAGE ����(�Ự��) 4xx ��Ӧ
        break;

    case EXOSIP_CALL_MESSAGE_SERVERFAILURE: // ��Ϣʧ�ܣ�������������
        // Method: MESSAGE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� MESSAGE ����(�Ự��) 5xx ��Ӧ
        break;

    case EXOSIP_CALL_MESSAGE_GLOBALFAILURE: // ��Ϣʧ�ܣ�ȫ��������
        // Method: MESSAGE
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� MESSAGE ����(�Ự��) 6xx ��Ӧ
        break;

    case EXOSIP_CALL_CLOSED:             // �����ѹرգ���������Ϊ����������ʱ
        // Method: BYE
        // Type: Request
        // Translate: �յ��ϼ������¼����͵� BYE ����
        break;

    case EXOSIP_CALL_RELEASED:           // �������ͷţ���Դ������
        // Method: NONE
        // Type: Event
        // Translate: �Ự�ͷ�
        break;

    case EXOSIP_MESSAGE_NEW:             // �յ��µ�SIP��Ϣ����
        // Method: MESSAGE, REGISTER, NOTIFY
        // Type: Request
        // Translate: �յ��ϼ������¼����͵� MESSAGE ��Ϣ
        break;

    case EXOSIP_MESSAGE_PROCEEDING:      // ��Ϣ���ڴ����У��ѷ�����Ӧ����δ���
        // Method: MESSAGE
        // Type: Response
        // Translate: ���ϼ������¼����͵� MESSAGE ���� 1xx ��Ӧ
        break;

    case EXOSIP_MESSAGE_ANSWERED:        // ��Ϣ�ѱ��ɹ���Ӧ�����磬�Է��ѻظ���
        // Method: MESSAGE
        // Type: Response
        // Translate: ���ϼ������¼����͵� MESSAGE ���� 2xx ��Ӧ
        break;

    case EXOSIP_MESSAGE_REDIRECTED:      // ��Ϣ���ض���������ַ��Ŀ��
        // Method: MESSAGE
        // Type: Response
        // Translate: ���ϼ������¼����͵� MESSAGE ���� 3xx ��Ӧ
        break;

    case EXOSIP_MESSAGE_REQUESTFAILURE:  // ��Ϣ����ʧ�ܣ����������ڿͻ��˴�����4xx��Ӧ��
        // Method: MESSAGE
        // Type: Response
        // Translate: ���ϼ������¼����͵� MESSAGE ���� 4xx ��Ӧ
        break;

    case EXOSIP_MESSAGE_SERVERFAILURE:   // ��Ϣ����ʧ�ܣ����������ڷ�����������5xx��Ӧ��
        // Method: MESSAGE
        // Type: Response
        // Translate: ���ϼ������¼����͵� MESSAGE ���� 5xx ��Ӧ
        break;

    case EXOSIP_MESSAGE_GLOBALFAILURE:   // ��Ϣʧ�ܣ�ȫ�������⣨�����粻�ɴ6xx��Ӧ��
        // Method: MESSAGE
        // Type: Response
        // Translate: ���ϼ������¼����͵� MESSAGE ���� 6xx ��Ӧ
        break;

    case EXOSIP_SUBSCRIPTION_NOANSWER:   // ��������δ����Ӧ
        // Method: NONE(SUBSCRIPTION)
        // Type: Event
        // Translate: ���¼�ƽ̨���͵� SUBSCRIBE/REFER ��������Ӧ
        break;

    case EXOSIP_SUBSCRIPTION_PROCEEDING: // �����������ڴ�����
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� SUBSCRIBE/REFER ���� 1xx ��Ӧ
        break;

    case EXOSIP_SUBSCRIPTION_ANSWERED:   // ���������ѱ���Ӧ
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� SUBSCRIBE/REFER ���� 2xx ��Ӧ
        break;

    case EXOSIP_SUBSCRIPTION_REDIRECTED: // ���������ض���
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� SUBSCRIBE/REFER ���� 3xx ��Ӧ
        break;

    case EXOSIP_SUBSCRIPTION_REQUESTFAILURE: // ��������ʧ��
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� SUBSCRIBE/REFER ���� 4xx ��Ӧ
        break;

    case EXOSIP_SUBSCRIPTION_SERVERFAILURE: // ����ʧ�ܣ�������������
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� SUBSCRIBE/REFER ���� 5xx ��Ӧ
        break;

    case EXOSIP_SUBSCRIPTION_GLOBALFAILURE: // ����ʧ�ܣ�ȫ��������
        // Method: SUBSCRIPTION
        // Type: Response
        // Translate: ���¼�ƽ̨���͵� SUBSCRIBE/REFER ���� 6xx ��Ӧ
        break;

    case EXOSIP_SUBSCRIPTION_NOTIFY:     // �յ�֪ͨ�����������и���
        // Method: NOTIFY
        // Type: Request
        // Translate: �յ��¼�ƽ̨���͵� NOTIFY ����
        break;

    case EXOSIP_IN_SUBSCRIPTION_NEW:     // �յ��µĶ�������
        // Method: SUBSCRIBE
        // Type: Request
        // Translate: �յ��ϼ�ƽ̨���͵� SUBSCRIBE/REFER ����
        break;

    case EXOSIP_NOTIFICATION_NOANSWER:   // ֪ͨδ����Ӧ
        // Method: NONE(NOTIFY)
        // Type: Event
        // Translate: ���ϼ�ƽ̨���͵� Notify ��������Ӧ
        break;

    case EXOSIP_NOTIFICATION_PROCEEDING: // ֪ͨ���ڴ�����
        // Method: NOTIFY
        // Type: Response
        // Translate: ���ϼ�ƽ̨���͵� Notify ���� 1xx ��Ӧ
        break;

    case EXOSIP_NOTIFICATION_ANSWERED:   // ֪ͨ�ѱ���Ӧ
        // Method: NOTIFY
        // Type: Response
        // Translate: ���ϼ�ƽ̨���͵� Notify ���� 2xx ��Ӧ
        break;

    case EXOSIP_NOTIFICATION_REDIRECTED: // ֪ͨ���ض���
        // Method: NOTIFY
        // Type: Response
        // Translate: ���ϼ�ƽ̨���͵� Notify ���� 3xx ��Ӧ
        break;

    case EXOSIP_NOTIFICATION_REQUESTFAILURE: // ֪ͨ����ʧ��
        // Method: NOTIFY
        // Type: Response
        // Translate: ���ϼ�ƽ̨���͵� Notify ���� 4xx ��Ӧ
        break;

    case EXOSIP_NOTIFICATION_SERVERFAILURE: // ֪ͨʧ�ܣ�������������
        // Method: NOTIFY
        // Type: Response
        // Translate: ���ϼ�ƽ̨���͵� Notify ���� 5xx ��Ӧ
        break;

    case EXOSIP_NOTIFICATION_GLOBALFAILURE: // ֪ͨʧ�ܣ�ȫ��������
        // Method: NOTIFY
        // Type: Response
        // Translate: ���ϼ�ƽ̨���͵� Notify ���� 6xx ��Ӧ
        break;

    case EXOSIP_EVENT_COUNT:             // �¼�������ͨ�������ڲ�ͳ��
        // Method: NONE
        // Type: None
        // Translate: �¼����ֵ
        break;

    default:                             // Ĭ�Ϸ�֧������δ֪�¼�����
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

    osip_message_t* pReg = NULL;
    int rid = eXosip_register_build_initial_register(m_pSipCtx, from, proxy, NULL, m_clientInfo.iSipExpiry, &pReg);
    if (rid < 0) {
        LOGI("eXosip_register_build_initial_register fail:%d", rid);
        return;
    }
    int ret = eXosip_register_send_register(m_pSipCtx, rid, pReg);
    if (OSIP_SUCCESS != ret) {
        LOGI("eXosip_register_send_register fail:%d", ret);
        return;
    }
    LOGI(">>> initial REGISTER rid=%d", rid);
    m_rid = rid;
}

void SipClient::Request_REGISTER_Authorization(osip_message_t* pResponse)
{
    if (!pResponse || pResponse->status_code != 401)
        return;
    /* ���� REGISTER�������֤��Ϣ���·��� */
    /* ȡ�� WWW-Authenticate ͷ */
    osip_www_authenticate_t* www = NULL;
    int iRet = osip_message_get_www_authenticate(pResponse, 0, &www);
    if (OSIP_SUCCESS != iRet) {
        LOGI("osip_message_get_www_authenticate fail:%d", iRet);
        return;
    }
    if (www && www->realm && www->nonce) {
        /* 1. ���¹��� REGISTER��ͬ rid�� */
        osip_message_t* pReg = NULL;
        iRet = eXosip_register_build_register(m_pSipCtx, m_rid, m_clientInfo.iSipExpiry, &pReg);
        if (OSIP_SUCCESS != iRet) {
            LOGI("eXosip_register_build_register fail:%d", iRet);
            return;
        }

        std::string method = "REGISTER"; 
        std::string username = m_clientInfo.sUser;
        std::string realm = osip_strdup_without_quote(www->realm);
        std::string nonce = osip_strdup_without_quote(www->nonce);
        std::string uri = "sip:" + m_clientInfo.sRegUri;
        std::string algorithm = "MD5";
        std::string cnonce = "cnonce123456789";
        std::string message_qop = "auth";
        std::string nonce_count = "00000001";
        std::string password = m_clientInfo.sPwd;
        //��ȡ�����еĽ�����Ϣ��hash
        HASHHEX hashResponse = "";
        {
            HASHHEX hash1 = "", hash2 = "";
            DigestCalcHA1(algorithm.c_str(), username.c_str(), realm.c_str(), password.c_str(), nonce.c_str(), nonce_count.c_str(), hash1);
            DigestCalcResponse(hash1, nonce.c_str(), nonce_count.c_str(), cnonce.c_str(), message_qop.c_str(), 0, method.c_str(), uri.c_str(), hash2, hashResponse);
            LOGI("hashCalc hash1=%s hash2=%s response=%s", hash1, hash2, hashResponse);
        }

        osip_authorization_t* pHeader = nullptr;
        osip_authorization_init(&pHeader);//����Authorization��Ӧͷ
        osip_authorization_set_auth_type(pHeader, osip_strdup("Digest"));
        osip_authorization_set_username(pHeader, osip_enquote(username.c_str()));
        osip_authorization_set_realm(pHeader, osip_enquote(realm.c_str()));
        osip_authorization_set_nonce(pHeader, osip_enquote(nonce.c_str()));
        osip_authorization_set_uri(pHeader, osip_enquote(uri.c_str()));
        osip_authorization_set_response(pHeader, osip_enquote(hashResponse));
        osip_authorization_set_algorithm(pHeader, osip_strdup(algorithm.c_str()));
        osip_authorization_set_cnonce(pHeader, osip_enquote(cnonce.c_str()));
        osip_authorization_set_message_qop(pHeader, osip_strdup(message_qop.c_str()));
        osip_authorization_set_nonce_count(pHeader, osip_strdup(nonce_count.c_str()));

        char* pDest = nullptr;
        osip_authorization_to_str(pHeader, &pDest);//����Ӧͷ������������ַ���
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