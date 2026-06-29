#include "http_server.h"

#include "nlohmann/json.hpp"
#include "tools/log.h"
#include "tools/config.h"
#include "gb28181_server.h"

using json = nlohmann::json;

namespace {

struct HttpResponse
{
    int code = 200;
    std::string msg = "success";
    json data = json::object();

    std::string ToJson() {
        json resp_json;
        resp_json["code"] = code;
        resp_json["msg"] = msg;
        resp_json["data"] = data;
        return resp_json.dump();
    }
};


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


json GetDeviceListValue(const std::map<std::string, gb28181::Device>& deviceList)
{
    json deviceListValue = json::array();
    for (auto deviceIter = deviceList.begin(); deviceIter != deviceList.end(); ++deviceIter)
    {
        gb28181::Device device = deviceIter->second;
        json deviceValue = json::object();
        deviceValue["deviceId"] = device.deviceId;
        deviceValue["channelNum"] = device.channelNum;

        json channelsValue = json::array();
        for (auto channelIter = device.channels.begin(); channelIter != device.channels.end(); ++channelIter) {
            gb28181::DeviceChannel channel = channelIter->second;
            json channelValue = json::object();
            channelValue["deviceId"] = channel.deviceId;
            channelValue["name"] = channel.name;
            channelValue["parentId"] = channel.parentId;
            channelValue["ip"] = channel.address;

            channelValue["status"] = channel.status;
            channelsValue.push_back(channelValue);
        }
        deviceValue["channels"] = channelsValue;
        deviceListValue.push_back(deviceValue);
    }
    return deviceListValue;
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
        std::string rtspUrl;
        if (shared) {
            ret = shared->OpenStream(info);
            if (ret >= 0) {
                std::string streamId = shared->CreateStreamId(info);
                std::string rtpIp = shared->GetConfig()->GetString("media", "ip");
                rtspUrl = "rtsp://" + rtpIp + ":554/rtp/" + streamId;
            }
        }
        HttpResponse resp;
        resp.code = ret;
        resp.data["rtsp"] = rtspUrl;
        std::string content = resp.ToJson();
        res.set_content(content, "application/json; charset=utf-8");
        });

    m_server.Post("/closeStream", [this](const httplib::Request& req, httplib::Response& res) {
        StreamInfo info = GetStreamInfo(req.body);
        std::shared_ptr<GB28181Server> shared = m_gbServer.lock();
        int ret = -1;
        if (shared) {
            ret = shared->CloseStream(info);
        }
        HttpResponse resp;
        resp.code = ret;
        std::string content = resp.ToJson();
        res.set_content(content, "application/json; charset=utf-8");
        });

    m_server.Post("/controlStream", [this](const httplib::Request& req, httplib::Response& res) {
        StreamInfo info = GetStreamInfo(req.body);
        std::shared_ptr<GB28181Server> shared = m_gbServer.lock();
        int ret = -1;
        if (shared) {
            ret = shared->ControlStream(info);
        }
        HttpResponse resp;
        resp.code = ret;
        std::string content = resp.ToJson();
        res.set_content(content, "application/json; charset=utf-8");
        });

    m_server.Post("/getDeviceList", [this](const httplib::Request& req, httplib::Response& res) {
        MessageInfo info = GetMessageInfo(req.body);
        std::shared_ptr<GB28181Server> shared = m_gbServer.lock();
        int ret = -1;
        json deviceListValue = json::array();
        if (shared) {
            ret = 0;
            auto deviceList = shared->GetDeviceList(info);
            deviceListValue = GetDeviceListValue(deviceList);
        }

        HttpResponse resp;
        resp.code = ret;
        resp.data = deviceListValue;
        std::string content = resp.ToJson();
        res.set_content(content, "application/json; charset=utf-8");

        });

    m_server.Post("/deviceControl", [this](const httplib::Request& req, httplib::Response& res) {
        ControlInfo info = GetControlInfo(req.body);
        std::shared_ptr<GB28181Server> shared = m_gbServer.lock();
        int ret = -1;
        json deviceListValue = json::array();
        if (shared) {
            ret = shared->DeviceControl(info);
        }

        HttpResponse resp;
        resp.code = ret;
        std::string content = resp.ToJson();
        res.set_content(content, "application/json; charset=utf-8");

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
    try
    {
        json root = json::parse(body);
        info.streamId = root.value("streamId", "");
        info.deviceId = root.value("deviceId", "");
        info.ip = root.value("ip", "");
        info.port = root.value("port", 0);
        info.channelId = root.value("channelId", "");
        info.streamNumber = root.value("streamNumber", 0);
        info.tcpMode = root.value("tcpMode", 0);
        info.startTime = root.value("startTime", "");
        info.endTime = root.value("endTime", "");

        info.controlType = root.value("controlType", 0);
        info.seekTime = root.value("seekTime", 0);
        info.speed = root.value("speed", 1.0);
    }
    catch (const std::exception& e)
    {

    }
    return info;
}

MessageInfo HttpServer::GetMessageInfo(const std::string& body)
{
    MessageInfo info;
    try
    {
        json root = json::parse(body);
        info.deviceId = root.value("deviceId", "");
        info.ip = root.value("ip", "");
        info.port = root.value("port", 0);
        info.update = root.value("update", false);
    }
    catch (const std::exception& e)
    {

    }
    return info;
}

ControlInfo HttpServer::GetControlInfo(const std::string& body)
{
    ControlInfo info;
    try
    {
        json root = json::parse(body);
        info.deviceId = root.value("deviceId", "");
        info.ip = root.value("ip", "");
        info.port = root.value("port", 0);
        info.controlType = (PTZControlType)root.value("controlType", 0);
        info.controlValue = root.value("controlValue", 0);
        return info;
    } catch (const std::exception& e)
    {

    }
    return info;
}


