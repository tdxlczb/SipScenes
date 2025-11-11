#pragma once
#include <string>
#include <memory>
#include "tools/httplib.h"
#include "common_def.h"

class GB28181Server;
class HttpServer
{
public:
	HttpServer();
	~HttpServer();

    bool Init(std::weak_ptr<GB28181Server> gbServer, const std::string& ip, int port);

    StreamInfo GetStreamInfo(const std::string& body);
private:
    httplib::Server m_server;
	std::weak_ptr<GB28181Server> m_gbServer;
};

