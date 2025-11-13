#pragma once
#include <string>

//字符串工具类
class StringUtils
{
public:
    static std::string GB2312ToUTF8(const std::string& gb2312Str);

    static std::string UTF8ToGB2312(const std::string& utf8Str);

private:

};
