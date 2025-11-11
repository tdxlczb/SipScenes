#include "time_utils.h"

#define _CRT_SECURE_NO_WARNINGS

// 秒级时间戳转字符串
std::string TimeUtils::secondsToString(time_t timestamp, const std::string& format) {
    std::tm localTime;
    localtime_s(&localTime, &timestamp);
    std::ostringstream oss;
    oss << std::put_time(&localTime, format.c_str());
    return oss.str();
}

// 毫秒级时间戳转字符串
std::string TimeUtils::millisecondsToString(long long milliseconds, const std::string& format) {
    time_t seconds = milliseconds / 1000;
    long long ms = milliseconds % 1000;

    std::tm localTime;
    localtime_s(&localTime, &seconds);
    std::ostringstream oss;
    oss << std::put_time(&localTime, format.c_str());
    oss << "." << std::setfill('0') << std::setw(3) << ms;

    return oss.str();
}

// 字符串转时间戳（秒）
time_t TimeUtils::stringToSeconds(const std::string& timeStr, const std::string& format) {
    std::tm tm = {};
    std::istringstream iss(timeStr);
    iss >> std::get_time(&tm, format.c_str());
    return std::mktime(&tm);
}

// 字符串转时间戳（毫秒）
long long TimeUtils::stringToMilliseconds(const std::string& timeStr, const std::string& format) {
    // 分离毫秒部分
    std::string timeWithoutMs = timeStr;
    long long milliseconds = 0;

    size_t dotPos = timeStr.find('.');
    if (dotPos != std::string::npos) {
        timeWithoutMs = timeStr.substr(0, dotPos);
        std::string msStr = timeStr.substr(dotPos + 1);
        milliseconds = std::stoll(msStr);

        // 确保毫秒部分是3位数
        if (msStr.length() == 1) milliseconds *= 100;
        else if (msStr.length() == 2) milliseconds *= 10;
    }

    // 解析时间字符串
    std::tm tm = {};
    std::istringstream iss(timeWithoutMs);
    iss >> std::get_time(&tm, format.c_str());

    if (iss.fail()) {
        throw std::runtime_error("时间字符串解析失败");
    }

    // 转换为时间戳（秒）
    time_t seconds = std::mktime(&tm);

    // 组合秒和毫秒
    return static_cast<long long>(seconds) * 1000 + milliseconds;
}

std::string TimeUtils::secondsChangeFormat(const std::string& timeStr, const std::string& srcFormat, const std::string& dstFormat)
{
    std::tm tm = {};
    std::istringstream iss(timeStr);
    iss >> std::get_time(&tm, srcFormat.c_str());
    std::ostringstream oss;
    oss << std::put_time(&tm, dstFormat.c_str());
    return oss.str();
}

// 获取当前时间戳（秒）
time_t TimeUtils::currentSeconds() {
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

// 获取当前时间戳（毫秒）
long long TimeUtils::currentMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}