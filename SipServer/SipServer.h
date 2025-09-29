#pragma once
extern "C" {
#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
}
#include <iostream>
#include <string>
#include <map>

struct ServerInfo {
    std::string sUa;//SIP����������
    std::string sNonce;//SIP���������ֵ
    std::string sIp;//SIP����IP
    int iPort = 0;//SIP����˿�
    std::string sSipId; //SIP������ID
    std::string sSipRealm;//SIP��������
    std::string sSipPass;//SIP password
    int iSipTimeout = 0; //SIP timeout
    int iSipExpiry = 0;// SIPĬ�ϵ���ʱ��:��
    int iRtpPort = 0; //SIP-RTP����˿�
};

struct ClientInfo {
    std::string sIp; // �ͻ���ip
    int iPort = 0; // �ͻ��˶˿�
    std::string sDevice;// 340200000013200000024
    bool isReg = false;//�Ƿ�ע��
    int iRtpPort = 0; //SIP-RTP�ͻ��˶˿�
};

class SipServer
{
public:
    SipServer();
    ~SipServer();
    /// <summary>
    /// ����Sip������������
    /// </summary>
    /// <param name="serverInfo">Sip��������Ϣ</param>
    /// <returns>�Ƿ���ز������ɹ�</returns>
    bool Init(const ServerInfo& serverInfo);
    /// <summary>
    /// �¼�����ѭ��
    /// </summary>
    void Loop();
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
    ServerInfo m_serverInfo;        //��������Ϣ
    struct eXosip_t* m_pSipCtx;//Sip������
    bool m_isRun = false;//�Ƿ������¼�ѭ��
    std::map<std::string, ClientInfo> m_mapClient;// <DeviceID,SipClient> �ͻ���map
};

