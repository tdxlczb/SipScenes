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
    std::string sRealm; // ��
    std::string sIp; // �ͻ���ip
    int iPort = 0; // �ͻ��˶˿�
    int iSipExpiry = 0; //SIPע�ᵽ��ʱ��:��

    std::string sRegUri;//ע��ķ���˵�ַ���� 172.16.19.108:5060 ���� sip.pjsip.org
};


class SipClient
{
public:
    SipClient();
    ~SipClient();
    /// <summary>
    /// ��ʼ��Sip�ͻ���
    /// </summary>
    /// <param name="serverInfo">Sip�ͻ�����Ϣ</param>
    /// <returns>�Ƿ��ʼ���ɹ�</returns>
    bool Init(const ClientInfo& clientInfo);
    /// <summary>
    /// �¼�����ѭ��
    /// </summary>
    void Loop();

    /// <summary>
    /// ע��
    /// </summary>
    void Register();

private:
    /// <summary>
    /// Sip�¼�����
    /// </summary>
    /// <param name="pSipEvt">Sip�¼�</param>
    void SipEventHandle(eXosip_event_t* pSipEvt);
    /// <summary>
    /// ��ʽ����ӡ��Ϣ
    /// </summary>
    /// <param name="pSipMsg">Sip��Ϣ</param>
    void DumpMessage(osip_message_t* pSipMsg);
    /// <summary>
    /// ��ʽ����ӡ����
    /// </summary>
    /// <param name="pSipEvt">Sip�¼�</param>
    void DumpRequest(eXosip_event_t* pSipEvt);
    /// <summary>
    /// ��ʽ����ӡ��Ӧ
    /// </summary>
    /// <param name="pSipEvt">Sip�¼�</param>
    void DumpResponse(eXosip_event_t* pSipEvt);
    /// <summary>
    /// ������sip_method = REGISTER
    /// </summary>
    /// <param name="pSipEvt">Sip�¼�</param>
    void Response_REGISTER(eXosip_event_t* pSipEvt);
    /// <summary>
    /// Response_REGISTER���Ӵ���(δ����Ȩ)
    /// </summary>
    /// <param name="pSipEvt">Sip�¼�</param>
    void Response_REGISTER_401unauthorized(eXosip_event_t* pSipEvt);
    /// <summary>
    /// ������sip_method = MESSAGE
    /// </summary>
    /// <param name="pSipEvt">Sip�¼�</param>
    void Response_MESSAGE(eXosip_event_t* pSipEvt);
    /// <summary>
    /// ������sip_method = INVITE
    /// </summary>
    /// <param name="pSipEvt">Sip�¼�</param>
    void Response_INVITE(eXosip_event_t* pSipEvt);
    /// <summary>
    /// ������Ӧ
    /// </summary>
    /// <param name="pSipEvt">Sip�¼�</param>
    /// <param name="iStatus">��Ӧ��</param>
    void MessageSendAnswer(eXosip_event_t* pSipEvt, int iStatus);
    /// <summary>
    /// ��������REGISTER
    /// </summary>
    /// <param name="clientInfo">���͵�Ŀ�Ķ���</param>
    void Request_REGISTER();
    /// <summary>
    /// ��������REGISTER Я����֤��Ϣ
    /// </summary>
    /// <param name="pResponse">Sip��Ӧ����Ϣ</param>
    void Request_REGISTER_Authorization(osip_message_t* pResponse);
    /// <summary>
    /// ��������INVITE
    /// </summary>
    /// <param name="clientInfo">���͵�Ŀ�Ķ���</param>
    void Request_INVITE(const ClientInfo& clientInfo);
    /// <summary>
    /// ��������MESSAGE
    /// </summary>
    /// <param name="clientInfo">���͵�Ŀ�Ķ���</param>
    void Request_MESSAGE(const ClientInfo& clientInfo);
    /// <summary>
    /// ����xml����ȡ������һ���ֵ
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
    ClientInfo m_clientInfo;   //�ͻ�����Ϣ
    struct eXosip_t* m_pSipCtx;//Sip������
    bool m_isRun = false;//�Ƿ������¼�ѭ��
    int m_rid = 0;//ע��id
    std::map<std::string, ClientInfo> m_mapClient;// <DeviceID,SipClient> �ͻ���map
};

