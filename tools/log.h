#pragma once
#include <time.h>
#include <string>

static std::string getTime() {
    static const char* sTimeFmt = "%Y-%m-%d %H:%M:%S";
    time_t tmVar = time(nullptr);
    char arrTime[64];
    tm stTm;
    localtime_s(&stTm, &tmVar);
    strftime(arrTime, sizeof(arrTime), sTimeFmt, &stTm);
    return arrTime;
}
//  __FILE__ 获取源文件的相对路径和名字
//  __LINE__ 获取该行代码在文件中的行号
//  __func__ 或 __FUNCTION__ 获取函数名

#define LOGI(format, ...)  fprintf(stderr,"[INFO]%s [%s:%d %s()] " format "\n", getTime().data(),__FILE__,__LINE__,__func__ ,##__VA_ARGS__)
#define LOGE(format, ...)  fprintf(stderr,"[ERROR]%s [%s:%d %s()] " format "\n",getTime().data(),__FILE__,__LINE__,__func__ ,##__VA_ARGS__)


#include <iostream>
#include <sstream>
#include <mutex>
#include <utility>   // std::move

class Logger {
public:
    static void setOutput(std::ostream& os) { s_out = &os; }

    // 工厂：返回可移动临时对象
    static Logger log() { return Logger(); }

    // 模板 << 支持任意可流式类型
    template<typename T>
    Logger& operator<<(const T& t) {
        m_oss << t;
        return *this;
    }

    // 支持 manipulator（如 std::endl）
    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        m_oss << manip;
        return *this;
    }

    // 关键：析构时输出 + 换行
    ~Logger() {
        std::lock_guard<std::mutex> lk(s_mutex);
        *s_out << m_oss.str() << '\n';
        s_out->flush();
    }

    // === 移动构造/赋值（C++11）===
    Logger(Logger&& other) noexcept : m_oss(std::move(other.m_oss)) {}
    Logger& operator=(Logger&&) noexcept = delete;

    // === 禁止拷贝 ===
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;   // 只允许工厂函数创建
    std::ostringstream m_oss;
    static std::mutex  s_mutex;
    static std::ostream* s_out;
};

//// 静态成员定义
//std::mutex Logger::s_mutex;
//std::ostream* Logger::s_out = &std::cout;

// 全局宏，方便使用
#define LOG_DEBUG   Logger::log()
#define LOG_INFO    Logger::log()
#define LOG_WARN    Logger::log()
#define LOG_ERROR   Logger::log()