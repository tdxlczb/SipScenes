#include "http_server.h"

#include "tools/log.h"
#include "tools/config.h"
#include "gb28181_server.h"

namespace {

std::string dump_headers(const httplib::Headers& headers) {
    std::string s;
    char buf[BUFSIZ];

    for (const auto& x : headers) {
        snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
        s += buf;
    }

    return s;
}

std::string dump_multipart_formdata(const httplib::MultipartFormData& form) {
    std::string s;
    char buf[BUFSIZ];

    s += "--------------------------------\n";

    for (const auto& x : form.fields) {
        const auto& name = x.first;
        const auto& field = x.second;

        snprintf(buf, sizeof(buf), "name: %s\n", name.c_str());
        s += buf;

        snprintf(buf, sizeof(buf), "text length: %zu\n", field.content.size());
        s += buf;

        s += "----------------\n";
    }

    for (const auto& x : form.files) {
        const auto& name = x.first;
        const auto& file = x.second;

        snprintf(buf, sizeof(buf), "name: %s\n", name.c_str());
        s += buf;

        snprintf(buf, sizeof(buf), "filename: %s\n", file.filename.c_str());
        s += buf;

        snprintf(buf, sizeof(buf), "content type: %s\n", file.content_type.c_str());
        s += buf;

        snprintf(buf, sizeof(buf), "text length: %zu\n", file.content.size());
        s += buf;

        s += "----------------\n";
    }

    return s;
}

std::string log(const httplib::Request& req, const httplib::Response& res) {
    std::string s;
    char buf[BUFSIZ];

    s += "================================\n";

    snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
        req.version.c_str(), req.path.c_str());
    s += buf;

    std::string query;
    for (auto it = req.params.begin(); it != req.params.end(); ++it) {
        const auto& x = *it;
        snprintf(buf, sizeof(buf), "%c%s=%s",
            (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
            x.second.c_str());
        query += buf;
    }
    snprintf(buf, sizeof(buf), "%s\n", query.c_str());
    s += buf;

    s += dump_headers(req.headers);
    s += dump_multipart_formdata(req.form);

    s += "--------------------------------\n";

    snprintf(buf, sizeof(buf), "%d\n", res.status);
    s += buf;
    s += dump_headers(res.headers);

    return s;
}

}

HttpServer::HttpServer()
{
}

HttpServer::~HttpServer()
{
}

bool HttpServer::Init(std::weak_ptr<GB28181Server> gbServer, const std::string& ip, int port)
{
    m_gbServer = gbServer;
    //
    m_server.Post("/openStream", [this](const httplib::Request& req, httplib::Response& res) {
        //auto body = dump_headers(req.headers) + dump_multipart_formdata(req.form);
        StreamInfo info = GetStreamInfo(req.body);
        std::shared_ptr<GB28181Server> shared = m_gbServer.lock();
        int ret = -1;
        Json::Value data = Json::ValueType::objectValue;
        if (shared) {
            ret = shared->OpenStream(info);
            if (ret >= 0) {
                std::string streamId = shared->CreateStreamId(info);
                std::string rtpIp = shared->GetConfig()->GetString("rtp_server", "ip");
                data["rtsp"] = "rtsp://" + rtpIp + ":554/rtp/" + streamId;
            }
        }
        Json::Value result;
        result["code"] = ret;
        result["msg"] = "";
        result["data"] = data;
        res.set_content(result.toStyledString(), "application/json; charset=utf-8");
        });

    m_server.Post("/closeStream", [this](const httplib::Request& req, httplib::Response& res) {
        StreamInfo info = GetStreamInfo(req.body);
        std::shared_ptr<GB28181Server> shared = m_gbServer.lock();
        int ret = -1;
        if (shared) {
            ret = shared->CloseStream(info);
        }
        Json::Value result;
        result["code"] = ret;
        result["msg"] = "";
        result["data"] = Json::ValueType::objectValue;
        res.set_content(result.toStyledString(), "application/json; charset=utf-8");
        });

    m_server.Post("/controlStream", [this](const httplib::Request& req, httplib::Response& res) {
        StreamInfo info = GetStreamInfo(req.body);
        std::shared_ptr<GB28181Server> shared = m_gbServer.lock();
        int ret = -1;
        if (shared) {
            ret = shared->ControlStream(info);
        }
        Json::Value result;
        result["code"] = ret;
        result["msg"] = "";
        result["data"] = Json::ValueType::objectValue;
        res.set_content(result.toStyledString(), "application/json; charset=utf-8");
        });

    m_server.Post("/getDeviceList", [this](const httplib::Request& req, httplib::Response& res) {
        MessageInfo info = GetMessageInfo(req.body);
        std::shared_ptr<GB28181Server> shared = m_gbServer.lock();
        int ret = -1;
        Json::Value deviceListValue = Json::arrayValue;
        if (shared) {
            ret = 0;
            auto deviceList = shared->GetDeviceList(info);
            deviceListValue = GetDeviceListValue(deviceList);
        }
        Json::Value result;
        result["code"] = ret;
        result["msg"] = "";
        result["data"] = deviceListValue;
        res.set_content(result.toStyledString(), "application/json; charset=utf-8");
        });

    m_server.set_error_handler([](const httplib::Request& /*req*/, httplib::Response& res) {
        const char* fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
        char buf[BUFSIZ];
        snprintf(buf, sizeof(buf), fmt, res.status);
        res.set_content(buf, "text/html");
        });

    m_server.set_logger(
        [](const httplib::Request& req, const httplib::Response& res) {
            std::string logstr = log(req, res);
            LOGE("%s", logstr.c_str());
        });

    auto base_dir = "./";
    if (!m_server.set_mount_point("/", base_dir)) {
        LOGE("The specified base directory doesn't exist.");
        return false;
    }

    if (m_server.listen(ip, port)) {
        LOGE("http server init failed.");
        return false;
    }
    return true;
}

StreamInfo HttpServer::GetStreamInfo(const std::string& body)
{
    StreamInfo info;
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(body, root)) {
        return info;
    }
    info.streamId = root.get("streamId", "").asString();
    info.deviceId = root.get("deviceId", "").asString();
    info.ip = root.get("ip", "").asString();
    info.port = root.get("port", 0).asInt();
    info.channelId = root.get("channelId", "").asString();
    info.streamNumber = root.get("streamNumber", 0).asInt();
    info.tcpMode = root.get("tcpMode", 0).asInt();
    info.startTime = root.get("startTime", "").asString();
    info.endTime = root.get("endTime", "").asString();

    info.controlType = root.get("controlType", 0).asInt();
    info.seekTime = root.get("seekTime", 0).asInt64();
    info.speed = root.get("speed", 1.0).asDouble();
    return info;
}

MessageInfo HttpServer::GetMessageInfo(const std::string& body)
{
    MessageInfo info;
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(body, root)) {
        return info;
    }
    info.deviceId = root.get("deviceId", "").asString();
    info.ip = root.get("ip", "").asString();
    info.port = root.get("port", 0).asInt();
    info.update = root.get("update", false).asBool();
    return info;
}

Json::Value HttpServer::GetDeviceListValue(const std::map<std::string, gb28181::Device>& deviceList)
{
    Json::Value deviceListValue = Json::arrayValue;
    for (auto deviceIter = deviceList.begin(); deviceIter != deviceList.end(); ++deviceIter)
    {
        gb28181::Device device = deviceIter->second;
        Json::Value deviceValue;
        deviceValue["deviceId"] = device.deviceId;
        deviceValue["channelNum"] = device.channelNum;

        Json::Value channelsValue = Json::arrayValue;     
        for (auto channelIter = device.channels.begin(); channelIter != device.channels.end(); ++channelIter) {
            gb28181::DeviceChannel channel = channelIter->second;
            Json::Value channelValue;
            channelValue["deviceId"] = channel.deviceId;
            channelValue["name"] = channel.name;
            channelValue["parentId"] = channel.parentId;
            channelValue["ip"] = channel.address;

            channelValue["status"] = channel.status;
            channelsValue.append(channelValue);
        }
        deviceValue["channels"] = channelsValue;
        deviceListValue.append(deviceValue);
    }
    return deviceListValue;
}
