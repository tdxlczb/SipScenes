#pragma once
#include <string>
#include <memory>
#include <thread>
#include "common_def.h"

class Config;
class HttpServer;
class SipServer;

class GB28181Server : public std::enable_shared_from_this<GB28181Server>
{
public:
    GB28181Server();
    ~GB28181Server();

    bool Start();
    int OpenStream(const StreamInfo& info);
    int CloseStream(const StreamInfo& info);
    int ControlStream(const StreamInfo& info);

    std::shared_ptr<Config> GetConfig();
    std::string CreateStreamId(const StreamInfo& info);
private:
    int OpenRtpServer(const std::string& streamId, int rtpPort, int tcpMode);
    int PauseRtpCheck(const std::string& streamId);
    int ResumeRtpCheck(const std::string& streamId);
    std::string CreateSSRC(bool isHistory, const std::string& realm);
    void Catalog();

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

    int m_streamSeq = 0;
};

