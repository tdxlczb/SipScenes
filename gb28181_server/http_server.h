#pragma once
#include <string>
#include <memory>
#include "cpp-httplib/httplib.h"
#include "common_def.h"
#include "gb28181/device.h"
#include "gb28181/device_channel.h"

class GB28181Server;
class HttpServer
{
public:
	HttpServer();
	~HttpServer();

    bool Init(std::weak_ptr<GB28181Server> gbServer, const std::string& ip, int port);

    StreamInfo GetStreamInfo(const std::string& body);
    MessageInfo GetMessageInfo(const std::string& body);

private:
    httplib::Server m_server;
	std::weak_ptr<GB28181Server> m_gbServer;
};

