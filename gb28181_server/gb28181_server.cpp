#include "gb28181_server.h"
#include <future>

#include "json/json.h"
#include "http_server.h"
#include "sip_server.h"
#include "tools/config.h"
#include "tools/file_utils.h"
#include "tools/time_utils.h"
#include "tools/log.h"
#include "gb28181/sdp.h"
#include "gb28181/message_xml.h"
#include "gb28181/stream.h"

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
    std::string rtpIp = m_config->GetString("rtp_server", "ip");
    int rtpPort = m_config->GetInt("rtp_server", "port");
    rtpPort = OpenRtpServer(streamId, rtpPort, info.tcpMode);
    if (rtpPort <= 0) {
        return -1;
    }
    std::string sipUser = m_config->GetString("sip_server", "user");
    std::string sipRealm = m_config->GetString("sip_server", "realm");

    gb28181::SdpParam sdpParam;
    sdpParam.deviceId = sipUser;
    sdpParam.channelId = info.channelId;
    sdpParam.ip = rtpIp;
    sdpParam.port = rtpPort;
    sdpParam.mode = info.tcpMode;
    if (!info.startTime.empty() && !info.endTime.empty()) {
        sdpParam.type = gb28181::kTransPlayBack;
        sdpParam.start = TimeUtils::stringToSeconds(info.startTime);
        sdpParam.end = TimeUtils::stringToSeconds(info.endTime);
        sdpParam.ssrc = CreateSSRC(true, sipRealm);
    }
    else {
        sdpParam.type = gb28181::kTransPlay;
        sdpParam.ssrc = CreateSSRC(false, sipRealm);
    }
    sdpParam.streamnumber = info.streamNumber;

    ClientInfo clientInfo;
    clientInfo.sUser = info.channelId;
    clientInfo.sIp = info.ip;
    clientInfo.iPort = info.port;
    InviteOptions options;
    options.sdp = gb28181::BuildInvateRequestSdp(sdpParam);
    options.subject = info.channelId + ":" + sdpParam.ssrc + "," + sipUser + ":0";
    return m_sipServer->Call(streamId, clientInfo, options);
}

int GB28181Server::CloseStream(const StreamInfo& info)
{
    std::string streamId = CreateStreamId(info);
    ClientInfo clientInfo;
    clientInfo.sUser = info.channelId;
    clientInfo.sIp = info.ip;
    clientInfo.iPort = info.port;
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

int GB28181Server::OpenRtpServer(const std::string& streamId, int rtpPort, int tcpMode)
{
    httplib::Client cli("localhost", 80);
    cli.set_connection_timeout(2, 0);
    char url[256];// = "/index/api/openRtpServer?port=0&secret=xZqp6Gi4W637Ns6WE2A0aiHUBeWeINah&stream_id=34020000001180000002_34020000001320000002_3&tcp_mode=0";
    snprintf(url, sizeof(url), "/index/api/openRtpServer?port=%d&secret=dDyYVky65wpSaEhr5kot1X9riUOA0MtN&stream_id=%s&tcp_mode=%d", rtpPort, streamId.c_str(), tcpMode);
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

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(res->body, root)) {
        return -1;
    }

    int code = root.get("code", -1).asInt();
    int port = root.get("port", -1).asInt();
    if (code == 0 && port > 0) {
        return port;
    }
    return -1;
}

int GB28181Server::PauseRtpCheck(const std::string& streamId)
{
    httplib::Client cli("localhost", 80);
    cli.set_connection_timeout(2, 0);
    char url[256];
    snprintf(url, sizeof(url), "/index/api/pauseRtpCheck?secret=dDyYVky65wpSaEhr5kot1X9riUOA0MtN&stream_id=%s", streamId.c_str());
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

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(res->body, root)) {
        return -1;
    }

    int code = root.get("code", -1).asInt();
    return code;
}

int GB28181Server::ResumeRtpCheck(const std::string& streamId)
{
    httplib::Client cli("localhost", 80);
    cli.set_connection_timeout(2, 0);
    char url[256];
    snprintf(url, sizeof(url), "/index/api/resumeRtpCheck?secret=dDyYVky65wpSaEhr5kot1X9riUOA0MtN&stream_id=%s", streamId.c_str());
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

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(res->body, root)) {
        return -1;
    }

    int code = root.get("code", -1).asInt();
    return code;
}

std::string GB28181Server::CreateSSRC(bool isHistory, const std::string& realm)
{
    m_streamSeq++;
    if (m_streamSeq > 9999)
        m_streamSeq = 1;

    std::stringstream ss;
    ss << std::setw(4) << std::setfill('0') << m_streamSeq;
    //第1位为历史或者实时流,0为实时，1为历史
    //第2-6位取监控域的4-8位
    //第7-10位为不充分的媒体流标识，这里使用自增，每生成一个ssrc都自增
    std::string ssrc = std::to_string((int)isHistory) + realm.substr(3, 5) + ss.str();
    return ssrc;
}

void GB28181Server::Catalog()
{
    std::promise<int> result;
    result.set_value(1);

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
    std::string sipIp = m_config->GetString("sip_server", "ip");
    int sipPort = m_config->GetInt("sip_server", "port");
    std::string sipUser = m_config->GetString("sip_server", "user");
    std::string sipPwd = m_config->GetString("sip_server", "pwd");
    std::string sipDomain = m_config->GetString("sip_server", "domain");
    std::string sipRealm = m_config->GetString("sip_server", "realm");

    ServerInfo info;
    info.sIp = sipIp;
    info.iPort = sipPort;
    info.sUser = sipUser;
    info.sPwd = sipPwd;
    info.sDomain = sipDomain;
    info.sRealm = sipRealm;

    if (!m_sipServer->Init(info))
        return false;

    m_sipThread = std::thread([this]() {
        m_sipServer->Loop();
        });
    return true;
}

bool GB28181Server::StartHttpServer()
{
    std::string httpIp = m_config->GetString("http_server", "ip");
    int httpPort = m_config->GetInt("http_server", "port");
    if (!m_httpServer->Init(GetWeakThis(), httpIp, httpPort))
        return false;
    return true;
}
