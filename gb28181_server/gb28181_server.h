#pragma once
#include <string>
#include <memory>
#include <thread>
#include <map>
#include "common_def.h"
#include "sip/sip_event.h"
#include "tools/thread_pool.h"
#include "gb28181/device.h"
#include "gb28181/device_channel.h"

using DeviceList = std::map<std::string, gb28181::Device>;

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

    DeviceList GetDeviceList(const MessageInfo& info);

    std::shared_ptr<Config> GetConfig();
    std::string CreateStreamId(const StreamInfo& info);
private:
    int OpenRtpServer(const std::string& streamId, int rtpPort, int tcpMode);
    int PauseRtpCheck(const std::string& streamId);
    int ResumeRtpCheck(const std::string& streamId);
    std::string CreateSSRC(bool isHistory, const std::string& realm);
    int CreateSN();
    // 返回sn号
    int QueryCatalog(const MessageInfo& info);
    int QueryDeviceInfo(const MessageInfo& info);

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

    DeviceList m_deviceList;
};

