#pragma once
#include <string>

class SipEvent
{
public:
	SipEvent() {};
	~SipEvent() {};

	virtual void OnRegister(const std::string& user) {};

private:

};
