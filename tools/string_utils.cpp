#include "string_utils.h"

#include <windows.h>
#include <string>
#include <iostream>

//GB2312转UTF8
std::string StringUtils::GB2312ToUTF8(const std::string& gb2312Str) {
    // GB2312 to Unicode
    int unicodeLen = MultiByteToWideChar(CP_ACP, 0, gb2312Str.c_str(), -1, nullptr, 0);
    if (unicodeLen == 0) return "";

    wchar_t* unicodeStr = new wchar_t[unicodeLen];
    MultiByteToWideChar(CP_ACP, 0, gb2312Str.c_str(), -1, unicodeStr, unicodeLen);

    // Unicode to UTF-8
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, unicodeStr, -1, nullptr, 0, nullptr, nullptr);
    if (utf8Len == 0) {
        delete[] unicodeStr;
        return "";
    }

    char* utf8Str = new char[utf8Len];
    WideCharToMultiByte(CP_UTF8, 0, unicodeStr, -1, utf8Str, utf8Len, nullptr, nullptr);

    std::string result(utf8Str);
    delete[] unicodeStr;
    delete[] utf8Str;

    return result;
}

//UTF8转GB2312
std::string StringUtils::UTF8ToGB2312(const std::string& utf8Str) {
    // UTF-8 to Unicode
    int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    if (unicodeLen == 0) return "";

    wchar_t* unicodeStr = new wchar_t[unicodeLen];
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, unicodeStr, unicodeLen);

    // Unicode to GB2312
    int gb2312Len = WideCharToMultiByte(CP_ACP, 0, unicodeStr, -1, nullptr, 0, nullptr, nullptr);
    if (gb2312Len == 0) {
        delete[] unicodeStr;
        return "";
    }

    char* gb2312Str = new char[gb2312Len];
    WideCharToMultiByte(CP_ACP, 0, unicodeStr, -1, gb2312Str, gb2312Len, nullptr, nullptr);

    std::string result(gb2312Str);
    delete[] unicodeStr;
    delete[] gb2312Str;

    return result;
}