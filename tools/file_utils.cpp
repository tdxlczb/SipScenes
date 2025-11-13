#include "file_utils.h"
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


// 获取当前工作目录
std::string ProcessPath::GetCurrentWorkingDirectory() {
    char buffer[4096];
    if (GETCWD(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    }
    return "";
}

// 获取可执行文件完整路径
std::string ProcessPath::GetExecutablePath() {
#ifdef _WIN32
    return getWindowsExecutablePath();
#else
    return getUnixExecutablePath();
#endif
}

// 获取可执行文件所在目录
std::string ProcessPath::GetExecutableDirectory() {
    std::string path = GetExecutablePath();
    return extractDirectory(path);
}

#ifdef _WIN32
std::string ProcessPath::getWindowsExecutablePath() {
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        return std::string(buffer);
    }

    // 备用方案：使用_getcwd
    char cwd[MAX_PATH];
    if (_getcwd(cwd, MAX_PATH) != nullptr) {
        return std::string(cwd) + "\\executable.exe"; // 需要结合argv[0]
    }

    return "";
}
#else
std::string ProcessPath::getUnixExecutablePath() {
    // 方法1: 使用 /proc/self/exe (Linux)
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length != -1) {
        buffer[length] = '\0';
        return std::string(buffer);
    }

    // 方法2: 使用 realpath 和 argv[0] (需要从main传入)
    return "";
}
#endif

std::string ProcessPath::extractDirectory(const std::string& path) {
    if (path.empty()) return "";

#ifdef _WIN32
    size_t pos = path.find_last_of("\\/");
#else
    size_t pos = path.find_last_of('/');
#endif

    if (pos != std::string::npos) {
        return path.substr(0, pos);
    }
    return path;
}
