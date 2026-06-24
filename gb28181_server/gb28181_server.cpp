#include "gb28181_server.h"
#include <future>
#include "nlohmann/json.hpp"
#include "http_server.h"
#include "sip/sip_server.h"
#include "tools/config.h"
#include "tools/file_utils.h"
#include "tools/time_utils.h"
#include "tools/string_utils.h"
#include "tools/log.h"
#include "gb28181/sdp.h"
#include "gb28181/manscdp.h"
#include "gb28181/stream.h"
#include "gb28181/tools.h"

using json = nlohmann::json;

GB28181Server::GB28181Server()
    : m_config(std::make_shared<Config>())
    , m_httpServer(std::make_shared<HttpServer>())
    , m_sipServer(std::make_shared<SipServer>())
{
    std::string configPath = ProcessPath::GetExecutableDirectory() + "/config.ini";
    m_config->Load(configPath);
}

GB28181Server::~GB28181Server()
{
    if (m_sipThread.joinable())
        m_sipThread.join();
}

bool GB28181Server::Start()
{
    if (!StartSipServer()) {
        return false;
    }

    if (!StartHttpServer()) {
        return false;
    }
    return true;
}

int GB28181Server::OpenStream(const StreamInfo& info)
{
    std::string streamId = CreateStreamId(info);
    int rtpPort = OpenRtpServer(streamId, info.tcpMode);
    if (rtpPort <= 0) {
        return -1;
    }
    std::string sipId = m_config->GetString("sip", "id");
    std::string rtpIp = m_config->GetString("media", "ip");
    gb28181::SdpParam sdpParam;
    sdpParam.deviceId = sipId;
    sdpParam.channelId = info.channelId;
    sdpParam.ip = rtpIp;
    sdpParam.port = rtpPort;
    sdpParam.mode = info.tcpMode;
    if (!info.startTime.empty() && !info.endTime.empty()) {
        sdpParam.type = gb28181::kTransPlayBack;
        sdpParam.start = TimeUtils::stringToSeconds(info.startTime);
        sdpParam.end = TimeUtils::stringToSeconds(info.endTime);
        sdpParam.ssrc = CreateSSRC(true, sipId);
    }
    else {
        sdpParam.type = gb28181::kTransPlay;
        sdpParam.ssrc = CreateSSRC(false, sipId);
    }
    sdpParam.streamnumber = info.streamNumber;

    ClientInfo clientInfo;
    clientInfo.id = info.channelId;
    clientInfo.ip = info.ip;
    clientInfo.port = info.port;
    InviteOptions options;
    options.sdp = gb28181::BuildInvateRequestSdp(sdpParam);
    options.subject = info.channelId + ":" + sdpParam.ssrc + "," + sipId + ":0";
    return m_sipServer->Call(streamId, clientInfo, options);
}

int GB28181Server::CloseStream(const StreamInfo& info)
{
    std::string streamId = CreateStreamId(info);
    ClientInfo clientInfo;
    clientInfo.id = info.channelId;
    clientInfo.ip = info.ip;
    clientInfo.port = info.port;
    return m_sipServer->Hangup(streamId, clientInfo);
}

int GB28181Server::ControlStream(const StreamInfo& info)
{
    std::string streamId = CreateStreamId(info);
    int cseq = static_cast<int>((rand() % 9 + 1) * pow(10, 8));
    std::string body;
    if (info.controlType == 1) {
        body = gb28181::BuildPlayPauseCmd(cseq);
        PauseRtpCheck(streamId);
    }
    else if (info.controlType == 2) {
        body = gb28181::BuildPlayResumeCmd(cseq);
        ResumeRtpCheck(streamId);
    }
    else if (info.controlType == 3) {
        body = gb28181::BuildPlaySeekCmd(cseq, info.seekTime);
    }
    else if (info.controlType == 4) {
        body = gb28181::BuildPlaySpeedCmd(cseq, info.speed);
    }
    else {
        return -1;
    }
    return m_sipServer->RequestInfo(streamId, body);
}

DeviceList GB28181Server::GetDeviceList(const MessageInfo& info)
{
    if (info.update) {
        m_threadPool.enqueue([this, info]() {
            QueryCatalog(info);
            });
    }
    return m_deviceList;
}

std::shared_ptr<Config> GB28181Server::GetConfig()
{
    return m_config;
}

std::string GB28181Server::CreateStreamId(const StreamInfo& info)
{
    if (!info.streamId.empty())
        return info.streamId;

    std::string streamId = info.deviceId + "_" + info.channelId + "_" + std::to_string(info.streamNumber) + "_" + std::to_string(info.tcpMode);
    if (!info.startTime.empty() && !info.endTime.empty()) {
        std::string start = TimeUtils::secondsChangeFormat(info.startTime, "%Y-%m-%d %H:%M:%S", "%Y%m%d%H%M%S");
        std::string end = TimeUtils::secondsChangeFormat(info.endTime, "%Y-%m-%d %H:%M:%S", "%Y%m%d%H%M%S");
        streamId.append("_").append(start).append("_").append(end);
    }
    return streamId;
}

int GB28181Server::OpenRtpServer(const std::string& streamId, int tcpMode)
{
    std::string ip = m_config->GetString("media", "ip");
    int port = m_config->GetInt("media", "port");
    std::string secret = m_config->GetString("media", "secret");
    int rtpPort = m_config->GetInt("media", "rtp_port");
    httplib::Client cli(ip, port);
    cli.set_connection_timeout(2, 0);
    char url[256];// = "/index/api/openRtpServer?port=0&secret=xZqp6Gi4W637Ns6WE2A0aiHUBeWeINah&stream_id=34020000001180000002_34020000001320000002_3&tcp_mode=0";
    snprintf(url, sizeof(url), "/index/api/openRtpServer?port=%d&secret=%s&stream_id=%s&tcp_mode=%d", rtpPort, secret.c_str(), streamId.c_str(), tcpMode);
    auto res = cli.Get(url);
    if (!res) {
        LOG_ERROR << "error code: " << res.error();
        return -1;
    }
    LOG_INFO << res->status;
    LOG_INFO << res->get_header_value("Content-Type");
    LOG_INFO << res->body;

    if (res->status != 200) {
        return -1;
    }

    try
    {
        json root = json::parse(res->body);
        int code = root.value("code", -1);
        rtpPort = root.value("port", -1);
        if (code == 0 && rtpPort > 0) {
            return rtpPort;
        }
    }
    catch (const std::exception& e)
    {
    }
    return -1;
}

int GB28181Server::PauseRtpCheck(const std::string& streamId)
{
    std::string ip = m_config->GetString("media", "ip");
    int port = m_config->GetInt("media", "port");
    std::string secret = m_config->GetString("media", "secret");
    httplib::Client cli(ip, port);
    cli.set_connection_timeout(2, 0);
    char url[256];
    snprintf(url, sizeof(url), "/index/api/pauseRtpCheck?secret=%s&stream_id=%s", secret.c_str(), streamId.c_str());
    auto res = cli.Get(url);
    if (!res) {
        LOG_ERROR << "error code: " << res.error();
        return -1;
    }
    LOG_INFO << res->status;
    LOG_INFO << res->get_header_value("Content-Type");
    LOG_INFO << res->body;

    if (res->status != 200) {
        return -1;
    }

    try
    {
        json root = json::parse(res->body);
        int code = root.value("code", -1);
        return code;
    }
    catch (const std::exception& e)
    {
    }
    return -1;
}

int GB28181Server::ResumeRtpCheck(const std::string& streamId)
{
    std::string ip = m_config->GetString("media", "ip");
    int port = m_config->GetInt("media", "port");
    std::string secret = m_config->GetString("media", "secret");
    httplib::Client cli(ip, port);
    cli.set_connection_timeout(2, 0);
    char url[256];
    snprintf(url, sizeof(url), "/index/api/resumeRtpCheck?secret=%s&stream_id=%s", secret.c_str(), streamId.c_str());
    auto res = cli.Get(url);
    if (!res) {
        LOG_ERROR << "error code: " << res.error();
        return -1;
    }
    LOG_INFO << res->status;
    LOG_INFO << res->get_header_value("Content-Type");
    LOG_INFO << res->body;

    if (res->status != 200) {
        return -1;
    }

    try
    {
        json root = json::parse(res->body);
        int code = root.value("code", -1);
        return code;
    }
    catch (const std::exception& e)
    {
    }
    return -1;
}

std::string GB28181Server::CreateSSRC(bool isHistory, const std::string& id)
{
    //这里使用自增，每生成一个ssrc都自增
    return gb28181::CreateSSRC(isHistory, id, ++m_streamSeq);
}

int GB28181Server::CreateSN()
{
    return ++m_messageSn;
}

int GB28181Server::QueryCatalog(const MessageInfo& info)
{
    gb28181::QueryCatalog query;
    query.CmdType = gb28181::kCatalog;
    query.SN = CreateSN();
    query.DeviceID = info.deviceId;
    std::string xml = gb28181::BuildQueryCatalog(query);

    ClientInfo clientInfo;
    clientInfo.id = info.deviceId;
    clientInfo.ip = info.ip;
    clientInfo.port = info.port;
    m_sipServer->RequestMessage(clientInfo, xml);

    return query.SN;
}

int GB28181Server::QueryDeviceInfo(const MessageInfo& info)
{
    gb28181::QueryDeviceInfo query;
    query.CmdType = gb28181::kDeviceInfo;
    query.SN = CreateSN();
    query.DeviceID = info.deviceId;
    std::string xml = gb28181::BuildQuery(query);

    ClientInfo clientInfo;
    clientInfo.id = info.deviceId;
    clientInfo.ip = info.ip;
    clientInfo.port = info.port;
    m_sipServer->RequestMessage(clientInfo, xml);

    return query.SN;
}

void GB28181Server::OnRegister(const ClientInfo& clientInfo)
{
    MessageInfo info;
    info.deviceId = clientInfo.id;
    info.ip = clientInfo.ip;
    info.port = clientInfo.port;
    m_threadPool.enqueue([this, info]() {
        QueryCatalog(info);
        });
}

void GB28181Server::OnMessage(const ClientInfo& info, const std::string& message)
{
    auto utf8Str = StringUtils::GB2312ToUTF8(message);
    gb28181::ResponseCatalog catalog = gb28181::GetResponseCatalog(utf8Str);
    if (catalog.DeviceID.empty())
        return;

    gb28181::Device newDevice = gb28181::GetDevice(catalog);
    if (m_deviceList.find(newDevice.deviceId) == m_deviceList.end()) {
        m_deviceList.emplace(newDevice.deviceId, newDevice);
        return;
    }
    auto deviceChannels = m_deviceList.at(newDevice.deviceId).channels;
    for (auto iter = newDevice.channels.begin(); iter != newDevice.channels.end(); ++iter)
    {
        if (deviceChannels.find(iter->first) != deviceChannels.end()) {
            //替换更新channel数据
            deviceChannels[iter->first] = iter->second;
        }
        else {
            deviceChannels.emplace(iter->first, iter->second);
        }
    }
    m_deviceList.at(newDevice.deviceId).channels = deviceChannels;
}

std::shared_ptr<GB28181Server> GB28181Server::GetSharedThis()
{
    return shared_from_this();
}

std::weak_ptr<GB28181Server> GB28181Server::GetWeakThis()
{
    //使用该函数时，对象必须由shared_ptr管理，即auto obj = std::make_shared<MyClass>()；不能在栈上创建对象，即MyClass obj
    return weak_from_this();
}

bool GB28181Server::StartSipServer()
{
    std::string sipIp = m_config->GetString("sip", "ip");
    int sipPort = m_config->GetInt("sip", "port");
    std::string sipId = m_config->GetString("sip", "id");
    std::string sipPwd = m_config->GetString("sip", "password");
    std::string sipDomain = m_config->GetString("sip", "domain");

    ServerInfo info;
    info.ip = sipIp;
    info.port = sipPort;
    info.id = sipId;
    info.password = sipPwd;
    info.domain = sipDomain;
    info.realm = sipDomain;

    if (!m_sipServer->Init(info))
        return false;
    m_sipServer->SetSipEvent(this);
    m_sipThread = std::thread([this]() {
        m_sipServer->Loop();
        });
    return true;
}

bool GB28181Server::StartHttpServer()
{
    std::string httpIp = m_config->GetString("http", "ip");
    int httpPort = m_config->GetInt("http", "port");
    if (!m_httpServer->Init(GetWeakThis(), httpIp, httpPort))
        return false;
    return true;
}
