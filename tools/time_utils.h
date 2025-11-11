#pragma once

#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>

class TimeUtils
{
public:
    // 秒级时间戳转字符串
    static std::string secondsToString(time_t timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S");

    // 毫秒级时间戳转字符串
    static std::string millisecondsToString(long long milliseconds, const std::string& format = "%Y-%m-%d %H:%M:%S");

    // 字符串转时间戳（秒）
    static time_t stringToSeconds(const std::string& timeStr, const std::string& format = "%Y-%m-%d %H:%M:%S");

    // 字符串转时间戳（毫秒秒）
    static long long stringToMilliseconds(const std::string& timeStr, const std::string& format = "%Y-%m-%d %H:%M:%S");

    static std::string secondsChangeFormat(const std::string& timeStr, const std::string& srcFormat, const std::string& dstFormat);

    // 获取当前时间戳（秒）
    static time_t currentSeconds();

    // 获取当前时间戳（毫秒）
    static long long currentMilliseconds();

};