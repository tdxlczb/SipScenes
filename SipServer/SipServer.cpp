#include "SipServer.h"
#include "Log.h"
#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <iphlpapi.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#include <Windows.h>

extern "C" {
#include "HTTPDigest.h"
}

///////�Ѻ��й��ܣ�ע�� ע�� ʵʱ�㲥

SipServer::SipServer()
	: m_pSipCtx(nullptr)
    , m_isRun(false)
{
}

SipServer::~SipServer()
{
    m_mapClient.clear();
}

bool SipServer::Init(const ServerInfo& serverInfo)
{
	m_serverInfo = serverInfo;
    m_pSipCtx = eXosip_malloc();
    if (!m_pSipCtx) {
        LOGE("eXosip_malloc error");
        return false;
    }
    if (eXosip_init(m_pSipCtx) != OSIP_SUCCESS) {
        LOGE("eXosip_init error");
        return false;
    }
    /*
    i = eXosip_listen_addr(ctx, IPPROTO_TCP, NULL, port, AF_INET, 0);  // TCP
    i = eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, port, AF_INET, 0);  // UDP
    i = eXosip_listen_addr(ctx, IPPROTO_TCP, NULL, port, AF_INET, 1);  // TLS
    i = eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, port, AF_INET, 1);  // DTLS
    */
    if (eXosip_listen_addr(m_pSipCtx, IPPROTO_UDP, nullptr, m_serverInfo.iPort, AF_INET, 0)) {
        LOGE("eXosip_listen_addr error");
        return false;
    }
    eXosip_set_user_agent(m_pSipCtx, m_serverInfo.sUa.c_str());
    if (eXosip_add_authentication_info(m_pSipCtx, m_serverInfo.sSipId.c_str(), m_serverInfo.sSipId.c_str()
        , m_serverInfo.sSipPass.c_str(), NULL, m_serverInfo.sSipRealm.c_str())) {
        LOGE("eXosip_add_authentication_info error");
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

void SipServer::SipEventHandle(eXosip_event_t* pSipEvt)
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
        break;

    case EXOSIP_REGISTRATION_FAILURE:    // ע��ʧ���¼���SIP�ͻ���ע��ʧ��
        // Method: REGISTER
        // Type: Response
        // Translate: �յ��ϼ�ƽ̨�� 3456xx ע��ʧ��
        break;

    case EXOSIP_CALL_INVITE:             // �յ�INVITE�������µĺ�������
        // Method: INVITE
        // Type: Request
        // Translate: �յ��ϼ�ƽ̨���͵� INVITE ����
#ifndef FIRST_SHOW
        this->DumpRequest(pSipEvt);
        this->DumpResponse(pSipEvt);
#endif // FIRST_SHOW
        this->Response_INVITE(pSipEvt);
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
#ifndef FIRST_SHOW
        this->DumpRequest(pSipEvt);
        this->DumpResponse(pSipEvt);
#endif // FIRST_SHOW
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
#ifndef FIRST_SHOW
        this->DumpRequest(pSipEvt);
        this->DumpResponse(pSipEvt);
#endif // FIRST_SHOW
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
#ifndef FIRST_SHOW
        this->DumpRequest(pSipEvt);
        this->DumpResponse(pSipEvt);
#endif // FIRST_SHOW
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
#ifndef FIRST_SHOW
        this->DumpRequest(pSipEvt);
        this->DumpResponse(pSipEvt);
#endif // FIRST_SHOW
        break;

    case EXOSIP_CALL_RELEASED:           // �������ͷţ���Դ������
        // Method: NONE
        // Type: Event
        // Translate: �Ự�ͷ�
#ifndef FIRST_SHOW
        this->DumpRequest(pSipEvt);
        this->DumpResponse(pSipEvt);
#endif // FIRST_SHOW
        //�����ͷ�����Client��Դ����������Ӧ�ý��ͷ�һ��Client
        m_mapClient.clear();
        break;

    case EXOSIP_MESSAGE_NEW:             // �յ��µ�SIP��Ϣ����
        // Method: MESSAGE, REGISTER, NOTIFY
        // Type: Request
        // Translate: �յ��ϼ������¼����͵� MESSAGE ��Ϣ
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

    case EXOSIP_MESSAGE_PROCEEDING:      // ��Ϣ���ڴ����У��ѷ�����Ӧ����δ���
        // Method: MESSAGE
        // Type: Response
        // Translate: ���ϼ������¼����͵� MESSAGE ���� 1xx ��Ӧ
        break;

    case EXOSIP_MESSAGE_ANSWERED:        // ��Ϣ�ѱ��ɹ���Ӧ�����磬�Է��ѻظ���
        // Method: MESSAGE
        // Type: Response
        // Translate: ���ϼ������¼����͵� MESSAGE ���� 2xx ��Ӧ
#ifndef FIRST_SHOW
        this->DumpRequest(pSipEvt);
#endif // FIRST_SHOW
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
#ifndef FIRST_SHOW
        this->DumpRequest(pSipEvt);
        this->DumpResponse(pSipEvt);
#endif // FIRST_SHOW
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

void SipServer::DumpMessage(osip_message_t* pSipMsg)
{
    char* pMsg;
    size_t szLen;
    osip_message_to_str(pSipMsg, &pMsg, &szLen);
    if (pMsg) {
        LOGI("\nprint message start\n%s\nprint message end\n", pMsg);
    }
}

void SipServer::DumpRequest(eXosip_event_t* pSipEvt)
{
    char* pMsg;
    size_t szLen;
    osip_message_to_str(pSipEvt->request, &pMsg, &szLen);
    if (pMsg) {
        LOGI("\nprint request start\ntype=%d\n%s\nprint request end\n", pSipEvt->type, pMsg);
    }
    //osip_free(pMsg);
}

void SipServer::DumpResponse(eXosip_event_t* pSipEvt)
{
    char* pMsg;
    size_t szLen;
    osip_message_to_str(pSipEvt->response, &pMsg, &szLen);
    if (pMsg) {
        LOGI("\nprint response start\ntype=%d\n%s\nprint response end\n", pSipEvt->type, pMsg);
    }
    //osip_free(pMsg);
}

void SipServer::Response_REGISTER(eXosip_event_t* pSipEvt)
{
    osip_authorization_t* pAuth = nullptr;
    osip_message_get_authorization(pSipEvt->request, 0, &pAuth);

    /*
    * ����ͷ��sip����������ע�����Ҫ��Ϣ������δЯ��Authorization�ֶ�����Ҫ����401��Ϣ��Я�������������Ը�֪�豸�ҷ���Ϣ
    REGISTER sip:SIP����������@Ŀ��������IP��ַ�˿� SIP/2.0
    Via: SIP/2.0/UDP Դ������IP��ַ�˿�
    From: <sip:SIP�豸����@Դ����>;tag=185326220
    To: <sip:SIP�豸����@Դ����>
    Call-ID: ms1214-322164710-681262131542511620107-0@172.18.16.3
    CSeq: 1 REGISTER
    Contact:<sip:SIP�豸����@ԴIP��ַ�˿�>
    Max-Forwards:70
    Expires:3600
    Content-Length:0
    */
    //���ȱ����ԴAuthorization��Ϣ�����ע��/ע����Ϣ��Ӧ����
    if (nullptr == pAuth || nullptr == pAuth->username) {
        Response_REGISTER_401unauthorized(pSipEvt);
        return;
    }

    /*
    * �����Authorization���������Ϣ��ȡ��У��
    J.1.3 Registersip:SIP����������@Ŀ��������IP��ַ�˿� SIP/2.0
    Via:SIP/2.0/UDP Դ������IP��ַ�˿�
    From:<sip:SIP�豸����@Դ����>;tag=185326220
    To:<sip:SIP�豸����@Դ����>
    Call-ID:ms1214-322164710-681262131542511620107-0@172.18.16.3
    CSeq:2Register
    Contact:<sip:SIP�豸����@ԴIP��ַ�˿�>
    Authorization:Digestusername="64010000002020000001",realm="64010000",nonce="6fe9
    ba44a76be22a",uri="sip:64010000002000000001@172.18.16.5:5060",response="9625d92d1bddea
    7a911926e0db054968",algorithm=MD5
    Max-Forwards:70
    Expires:3600
    Content-Length:0
    */

    char* method = NULL, // REGISTER
        * algorithm = NULL, // MD5
        * username = NULL,// 340200000013200000024 SIP�û���
        * realm = NULL, // sip�����������ͻ��ˣ��ͻ���Я�����ύ������sip������
        * nonce = NULL, // sip�����������ͻ��ˣ��ͻ���Я�����ύ������nonce
        * nonce_count = NULL,
        * uri = NULL, // sip:34020000002000000001@3402000000
        * response = NULL, // �ͻ��˼������ɵ���֤�ַ���
        * cnonce = NULL, // �ͻ������ɵ������
        * message_qop = NULL; // sip:34020000002000000001@3402000000

    method = pSipEvt->request->sip_method;

#define SIP_strdup(FIELD) if (pAuth->FIELD) (FIELD) = osip_strdup_without_quote(pAuth->FIELD);//��ȡ�ַ������������е�˫����
    SIP_strdup(algorithm)
    SIP_strdup(username);
    SIP_strdup(realm);
    SIP_strdup(nonce);
    SIP_strdup(nonce_count);
    SIP_strdup(uri);
    SIP_strdup(response);
    SIP_strdup(cnonce);
    SIP_strdup(message_qop);
#undef SIP__strdup

    //��ȡ�����еĽ�����Ϣ��hash
    HASHHEX hashResponse = "";
    {
        //������Ҫʹ�ÿͻ��˵��˺�������֤������һ���������Ҫ��sipЭ�������ڷ����ע���¼���˺�������Ϣ
        HASHHEX hash1 = "", hash2 = "";
        DigestCalcHA1(algorithm, username, realm, username, nonce, nonce_count, hash1);
        DigestCalcResponse(hash1, nonce, nonce_count, cnonce, message_qop, 0, method, uri, hash2, hashResponse);
        LOGI("hashCalc hash1=%s hash2=%s response=%s", hash1, hash2, hashResponse);
    }

    //��ȡ�����˿��û�����Ϣ
    osip_contact_t* pContact = nullptr;
    osip_message_get_contact(pSipEvt->request, 0, &pContact);
    //hash��֤����֤������Ϣһ����
    if (0 == memcmp(hashResponse, response, HASHHEXLEN)) {//һ����ע��/ע�����û�

        int iExpires = -1;
        osip_header_t* stExpires = nullptr;
        osip_message_get_expires(pSipEvt->request, 0, &stExpires);
        //��ע��
        LOGI("Expires:%s,%s", stExpires->hname, stExpires->hvalue);
        if (0 == atoi(stExpires->hvalue)) {
            LOGI("Camera unregistration success,ip=%s,port=%d,device=%s", pContact->url->host, atoi(pContact->url->port), _strdup(username));
            m_mapClient.erase(_strdup(username));
            LOGI("Camera num:%llu", m_mapClient.size());
            this->MessageSendAnswer(pSipEvt, 200);//֪ͨ����ͷע��ͨ��
        }
        else {
            LOGI("Camera registration success,ip=%s,port=%d,device=%s", pContact->url->host, atoi(pContact->url->port), _strdup(username));
            ClientInfo clientInfo{ _strdup(pContact->url->host), atoi(pContact->url->port), _strdup(username),false,0 };
            m_mapClient.insert(std::make_pair(clientInfo.sDevice, clientInfo));
            LOGI("Camera num:%llu", m_mapClient.size());
            this->MessageSendAnswer(pSipEvt, 200);//֪ͨ����ͷע��ͨ��
            //this->Request_INVITE(pstClientInfo);
            //this->Request_MESSAGE(clientInfo);
        }
    }
    else {//���������
        LOGI("Camera registration error, ip=%s,port=%d,device=%s", pContact->url->host, atoi(pContact->url->port), _strdup(username));
        //this->MessageSendAnswer(pSipEvt, 401);//֪ͨ����ͷע��/ע��ʧ��
    }

    osip_free(algorithm);
    osip_free(username);
    osip_free(realm);
    osip_free(nonce);
    osip_free(nonce_count);
    osip_free(uri);
}

void SipServer::Response_REGISTER_401unauthorized(eXosip_event_t* pSipEvt)
{
    /*
    * ������Ҫ����һ�����µ���Ϣ�壬������ҪЯ����ȥ����Ϣ��Ҫ�ǣ�WWW-Authenticate:Digestrealm="64010000",nonce="6fe9ba44a76be22a"
    J.1.2 SIP/2.0401Unauthorized
    To:sip:SIP�豸����@Դ����
    Content-Length:0
    CSeq:1Register
    Call-ID:ms1214-322164710-681262131542511620107-0@172.18.16.3
    From:<sip:SIP�豸����@Դ����>;tag=185326220
    Via:SIP/2.0/UDPԴ������IP��ַ�˿�
    WWW-Authenticate:Digestrealm="64010000",nonce="6fe9ba44a76be22a"
    */
    char* pDest = nullptr;
    osip_message_t* pMsg = nullptr;
    osip_www_authenticate_t* pHeader = nullptr;
    osip_www_authenticate_init(&pHeader);//����WWW-Authenticate��Ӧͷ
    osip_www_authenticate_set_auth_type(pHeader, osip_strdup("Digest"));//������֤����
    osip_www_authenticate_set_realm(pHeader, osip_enquote(m_serverInfo.sSipRealm.c_str()));//�ṩ��֤�õ�SIP��������
    osip_www_authenticate_set_nonce(pHeader, osip_enquote(m_serverInfo.sNonce.c_str()));//�ṩ��֤�õ�SIP���������ֵ
    osip_www_authenticate_set_algorithm(pHeader, osip_strdup("MD5"));
    osip_www_authenticate_set_qop_options(pHeader, osip_enquote("auth"));//����Ҫ��˫����
    osip_www_authenticate_to_str(pHeader, &pDest);//����Ӧͷ������������ַ���
    int iRet = eXosip_message_build_answer(m_pSipCtx, pSipEvt->tid, 401, &pMsg);//����һ����Ӧ
    if (OSIP_SUCCESS != iRet || !pMsg) {
        LOGE("eXosip_message_build_answer failed:%d", iRet);
    }

    iRet = osip_message_set_www_authenticate(pMsg, pDest);//����Ӧͷ�ַ�����ӵ���Ӧ��Ϣ��
    if (OSIP_SUCCESS != iRet) {
        LOGE("osip_message_set_www_authenticate failed:%d", iRet);
    }

    iRet = osip_message_set_content_type(pMsg, "Application/MANSCDP+xml");//������Ӧ��Ϣ����������
    if (OSIP_SUCCESS != iRet) {
        LOGE("osip_message_set_content_type failed:%d", iRet);
    }

    this->DumpMessage(pMsg);

    eXosip_lock(m_pSipCtx);
    iRet = eXosip_message_send_answer(m_pSipCtx, pSipEvt->tid, 401, pMsg);//������Ӧ
    eXosip_unlock(m_pSipCtx);
    if (OSIP_SUCCESS != iRet) {
        LOGE("eXosip_message_send_answer failed:%d", iRet);
    }
    LOGI("response_register_401unauthorized success");

    //������Դ
    osip_www_authenticate_free(pHeader);
    osip_free(pDest);
}

void SipServer::Response_MESSAGE(eXosip_event_t* pSipEvt)
{
    osip_body_t* pBody = nullptr;
    char sCmdType[64] = { 0 };
    char sDeviceID[64] = { 0 };
    osip_message_get_body(pSipEvt->request, 0, &pBody);
    if (pBody) {
        parseXml(pBody->body, "<CmdType>", false, "</CmdType>", false, sCmdType);
        parseXml(pBody->body, "<DeviceID>", false, "</DeviceID>", false, sDeviceID);
    }

    //    Client *client = getClientByDevice(DeviceID);
    //    if(client){
    //        LOGI("response_message��%s ��ע��",DeviceID);
    //    }else{
    //        LOGE("response_message��%s δע��",DeviceID);
    //    }
    LOGI("CmdType=%s,DeviceID=%s", sCmdType, sDeviceID);

    if (0 == strcmp(sCmdType, "Keepalive")) {//����һ����������������Ļ�Ӧһ��200��ok
        this->MessageSendAnswer(pSipEvt, 200);
    }
    else if (0 == strcmp(sCmdType, "Catalog")) {//?
        this->MessageSendAnswer(pSipEvt, 200);
    }
    else if (0 == strcmp(sCmdType, "Broadcast")) {//?
        this->MessageSendAnswer(pSipEvt, 200);
    }
    else {//������Ϣ
        this->MessageSendAnswer(pSipEvt, 200);
    }
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
        "f=v/////a/1/8/1\r\n", m_serverInfo.sSipId.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.sIp.c_str(), /*m_serverInfo.iRtpPort*/atoi(srcPort.c_str()));

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

void SipServer::MessageSendAnswer(eXosip_event_t* pSipEvt, int iStatus)
{
    int iRet = 0;
    osip_message_t* pMsg = nullptr;
    iRet = eXosip_message_build_answer(m_pSipCtx, pSipEvt->tid, iStatus, &pMsg);
    if (iRet == 0 && pMsg != nullptr)
    {
        eXosip_lock(m_pSipCtx);
        eXosip_message_send_answer(m_pSipCtx, pSipEvt->tid, iStatus, pMsg);//������Ӧ
        eXosip_unlock(m_pSipCtx);
    }
    else {
        LOGE("MessageSendAnswer error iStatus=%d,iRet=%d,pMsg=%d", iStatus, iRet, pMsg != nullptr);
    }
}

void SipServer::Request_INVITE(const ClientInfo& clientInfo)
{
    LOGI("INVITE");

    char sSessionExp[1024] = { 0 };
    osip_message_t* pMsg = nullptr;
    char sFrom[1024] = { 0 };
    char sTo[1024] = { 0 };
    char sContact[1024] = { 0 };
    char sSdp[2048] = { 0 };
    char sHead[1024] = { 0 };


    sprintf_s(sFrom, "sip:%s@%s:%d", m_serverInfo.sSipId.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.iPort); //<sip:ý�����������豸���� @Դ����>; tag = e3719a0b
    sprintf_s(sContact, "sip:%s@%s:%d", m_serverInfo.sSipId.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.iPort);//<sip:ý�����������豸����@ԴIP��ַ�˿�>
    sprintf_s(sTo, "sip:%s@%s:%d", clientInfo.sDevice.c_str(), clientInfo.sIp.c_str(), clientInfo.iPort);//��ý���������ߵ���Ϣ(�˴��豸���ͣ������豸��Ϣ
    sprintf_s(sSdp, 2048,
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
        "f=\r\n", m_serverInfo.sSipId.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.iRtpPort);


    //sprintf_s(sSdp, 2048,
    //    "v=0\r\n"
    //    "o=%s 0 0 IN IP4 %s\r\n"
    //    "s=Play\r\n"
    //    "c=IN IP4 %s\r\n"
    //    "t=0 0\r\n"
    //    "m=audio %d TCP/RTP/AVP 8\r\n"
    //    "a=sendonly\r\n"
    //    "a=rtpmap:8 PCMA/8000\r\n"
    //    "a=setup:passive\r\n"
    //    "a=connection:new\r\n"
    //    "y=0100000001\r\n"
    //    "f=v/////a/1/8/1\r\n", m_serverInfo.sSipId.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.sIp.c_str(), /*m_serverInfo.iRtpPort*/12095);

    int iRet = eXosip_call_build_initial_invite(m_pSipCtx, &pMsg, sTo, sFrom, nullptr, nullptr);
    if (iRet) {
        LOGE("eXosip_call_build_initial_invite error: %s %s ret:%d", sFrom, sTo, iRet);
        return;
    }

    osip_message_set_body(pMsg, sSdp, strlen(sSdp));
    osip_message_set_content_type(pMsg, "application/sdp");
    snprintf(sSessionExp, sizeof(sSessionExp) - 1, "%i;refresher=uac", m_serverInfo.iSipTimeout);
    osip_message_set_header(pMsg, "Session-Expires", sSessionExp);
    osip_message_set_supported(pMsg, "timer");

    int iCallId = eXosip_call_send_initial_invite(m_pSipCtx, pMsg);

    if (iCallId > 0) {
        LOGI("eXosip_call_send_initial_invite success: iCallId=%d", iCallId);
    }
    else {
        LOGE("eXosip_call_send_initial_invite error: iCallId=%d", iCallId);
    }
}

void SipServer::Request_MESSAGE(const ClientInfo& clientInfo)
{
    LOGI("MESSAGE");

    char sSessionExp[1024] = { 0 };
    osip_message_t* pMsg = nullptr;
    char sFrom[1024] = { 0 };
    char sTo[1024] = { 0 };
    char sContact[1024] = { 0 };
    char sXml[2048] = { 0 };
    char sHead[1024] = { 0 };


    sprintf_s(sFrom, "sip:%s@%s:%d", m_serverInfo.sSipId.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.iPort); //<sip:ý�����������豸���� @Դ����>; tag = e3719a0b
    sprintf_s(sContact, "sip:%s@%s:%d", m_serverInfo.sSipId.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.iPort);//<sip:ý�����������豸����@ԴIP��ַ�˿�>
    sprintf_s(sTo, "sip:%s@%s:%d", clientInfo.sDevice.c_str(), clientInfo.sIp.c_str(), clientInfo.iPort);//��ý���������ߵ���Ϣ(�˴��豸���ͣ������豸��Ϣ

    sprintf_s(sXml, 2048,
        "<? xmlversion=\"1.0\" ?>\r\n"
        "<Notify>\r\n"
        "<CmdType>Broadcast</CmdType>\r\n"
        "<SN>992</SN>\r\n"
        "<SourceID>34020000002000000001</SourceID>\r\n"
        "<TargetID>34020000001370000012</TargetID>\r\n"
        //"<TargetID>34020000001370000001</TargetID>\r\n"
        "</Notify>\r\n"
    );


    //sprintf_s(sSdp, 2048,
    //    "v=0\r\n"
    //    "o=%s 0 0 IN IP4 %s\r\n"
    //    "s=Play\r\n"
    //    "c=IN IP4 %s\r\n"
    //    "t=0 0\r\n"
    //    "m=audio %d TCP/RTP/AVP 8\r\n"
    //    "a=sendonly\r\n"
    //    "a=rtpmap:8 PCMA/8000\r\n"
    //    "a=setup:passive\r\n"
    //    "a=connection:new\r\n"
    //    "y=0100000001\r\n"
    //    "f=v/////a/1/8/1\r\n", m_serverInfo.sSipId.c_str(), m_serverInfo.sIp.c_str(), m_serverInfo.sIp.c_str(), /*m_serverInfo.iRtpPort*/12095);

    
    int iRet = eXosip_message_build_request(m_pSipCtx, &pMsg, "MESSAGE", sTo, sFrom, nullptr);
    if (iRet) {
        LOGE("eXosip_call_build_initial_invite error: %s %s ret:%d", sFrom, sTo, iRet);
        return;
    }
    osip_message_set_body(pMsg, sXml, strlen(sXml));
    osip_message_set_content_type(pMsg, "Application/MANSCDP+xml");//����������Ϣ����������

    int iCallId = eXosip_message_send_request(m_pSipCtx, pMsg);

    if (iCallId > 0) {
        LOGI("eXosip_message_send_request success: iCallId=%d", iCallId);
    }
    else {
        LOGE("eXosip_message_send_request error: iCallId=%d", iCallId);
    }
    /*
    char* pDest = nullptr;
    osip_message_t* pMsg = nullptr;
    osip_www_authenticate_t* pHeader = nullptr;

    osip_www_authenticate_init(&pHeader);//����WWW-Authenticate��Ӧͷ
    osip_www_authenticate_set_auth_type(pHeader, osip_strdup("Digest"));//������֤����
    osip_www_authenticate_set_realm(pHeader, osip_enquote(m_serverInfo.sSipRealm.c_str()));//�ṩ��֤�õ�SIP��������
    osip_www_authenticate_set_nonce(pHeader, osip_enquote(m_serverInfo.sNonce.c_str()));//�ṩ��֤�õ�SIP���������ֵ
    osip_www_authenticate_to_str(pHeader, &pDest);//����Ӧͷ������������ַ���
    int iRet = eXosip_message_build_answer(m_pSipCtx, pSipEvt->tid, 401, &pMsg);//����һ����Ӧ
    if (iRet == 0 && pMsg != nullptr) {
        osip_message_set_www_authenticate(pMsg, pDest);//����Ӧͷ�ַ�����ӵ���Ӧ��Ϣ��
        osip_message_set_content_type(pMsg, "Application/MANSCDP+xml");//������Ӧ��Ϣ����������

        this->DumpMessage(pMsg);

        eXosip_lock(m_pSipCtx);
        eXosip_message_send_answer(m_pSipCtx, pSipEvt->tid, 401, pMsg);//������Ӧ
        eXosip_unlock(m_pSipCtx);
        LOGI("response_register_401unauthorized success");
    }
    else {
        LOGE("response_register_401unauthorized error");
    }
    */
}

bool SipServer::parseXml(const char* pData, const char* pSMark, bool isWithSMake, const char* pEMark, bool isWithEMake, char* pDest)
{
    const char* satrt = strstr(pData, pSMark);

    if (satrt != NULL) {
        const char* end = strstr(satrt, pEMark);

        if (end != NULL) {
            int s_pos = isWithSMake ? 0 : strlen(pSMark);
            int e_pos = isWithEMake ? strlen(pEMark) : 0;

            strncpy_s(pDest, 64, satrt + s_pos, (end + e_pos) - (satrt + s_pos));
        }
        return true;
    }
    return false;
}
