#pragma once

#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define GETCWD _getcwd
#else
#include <unistd.h>
#include <linux/limits.h>
#define GETCWD getcwd
#endif

class ProcessPath {
public:
    // 获取当前工作目录
    static std::string GetCurrentWorkingDirectory();

    // 获取可执行文件完整路径
    static std::string GetExecutablePath();

    // 获取可执行文件所在目录
    static std::string GetExecutableDirectory();

private:
#ifdef _WIN32
    static std::string getWindowsExecutablePath();
#else
    static std::string getUnixExecutablePath();
#endif

    static std::string extractDirectory(const std::string& path);
};