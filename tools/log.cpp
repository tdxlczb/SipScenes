#include "log.h"


// 静态成员定义
std::mutex Logger::s_mutex;
std::ostream* Logger::s_out = &std::cout;