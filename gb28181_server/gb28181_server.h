#pragma once
#include <string>
#include <memory>
#include <thread>
#include "common_def.h"
#include "sip/sip_event.h"
#include "tools/thread_pool.h"

class Config;
class HttpServer;
class SipServer;

class GB28181Server : public std::enable_shared_from_this<GB28181Server>, public SipEvent
{
public:
    GB28181Server();
    ~GB28181Server();

    bool Start();
    int OpenStream(const StreamInfo& info);
    int CloseStream(const StreamInfo& info);
    int ControlStream(const StreamInfo& info);
    int GetDeviceList(const MessageInfo& info);

    std::shared_ptr<Config> GetConfig();
    std::string CreateStreamId(const StreamInfo& info);
private:
    int OpenRtpServer(const std::string& streamId, int rtpPort, int tcpMode);
    int PauseRtpCheck(const std::string& streamId);
    int ResumeRtpCheck(const std::string& streamId);
    std::string CreateSSRC(bool isHistory, const std::string& realm);
    int CreateSN();
    void QueryCatalog(const MessageInfo& info);
    void QueryDeviceInfo(const MessageInfo& info);

private:
    virtual void OnRegister(const ClientInfo& info) override;
    virtual void OnMessage(const ClientInfo& info, const std::string& message) override;
private:
    std::shared_ptr<GB28181Server> GetSharedThis();
    std::weak_ptr<GB28181Server> GetWeakThis();
    bool StartSipServer();
    bool StartHttpServer();

private:
    std::shared_ptr<Config> m_config;
    std::shared_ptr<HttpServer> m_httpServer;
    std::shared_ptr<SipServer> m_sipServer;
    
    std::thread  m_sipThread;
    ThreadPool m_threadPool;

    int m_streamSeq = 0;
    int m_messageSn = 0;
};

