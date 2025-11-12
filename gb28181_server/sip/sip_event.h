#pragma once
#include <string>
#include "sip_define.h"

class SipEvent
{
public:
	SipEvent() {};
	~SipEvent() {};

	virtual void OnRegister(const ClientInfo& info) {};
    virtual void OnMessage(const ClientInfo& info, const std::string& message) {};

private:

};
