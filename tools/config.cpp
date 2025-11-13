#include "config.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

bool Config::Load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::string current_section = "global";

    while (std::getline(file, line)) {
        line = trim(line);

        // 跳过空行和注释
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // 处理节(section)
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            current_section = trim(line.substr(1, line.length() - 2));
            continue;
        }

        // 处理键值对
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));
            m_data[current_section][key] = value;
        }
    }

    file.close();
    return true;
}

std::string Config::GetString(const std::string& section, const std::string& key, const std::string& default_value) {
    if (m_data.find(section) != m_data.end() &&
        m_data[section].find(key) != m_data[section].end()) {
        return m_data[section][key];
    }
    return default_value;
}

int Config::GetInt(const std::string& section, const std::string& key, int default_value) {
    std::string value = GetString(section, key, "");
    if (value.empty()) return default_value;

    try {
        return std::stoi(value);
    }
    catch (...) {
        return default_value;
    }
}

double Config::GetDouble(const std::string& section, const std::string& key, double default_value) {
    std::string value = GetString(section, key, "");
    if (value.empty()) return default_value;

    try {
        return std::stod(value);
    }
    catch (...) {
        return default_value;
    }
}

bool Config::GetBool(const std::string& section, const std::string& key, bool default_value) {
    std::string value = GetString(section, key, "");
    if (value.empty()) return default_value;

    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);

    if (lower_value == "true" || lower_value == "1" || lower_value == "yes") {
        return true;
    }
    else if (lower_value == "false" || lower_value == "0" || lower_value == "no") {
        return false;
    }
    return default_value;
}

// 显示所有配置（用于调试）
void Config::Display() {
    for (const auto& section : m_data) {
        std::cout << "[" << section.first << "]" << std::endl;
        for (const auto& item : section.second) {
            std::cout << item.first << " = " << item.second << std::endl;
        }
        std::cout << std::endl;
    }
}

// 去除字符串两端的空白字符
std::string Config::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return str.substr(start, end - start + 1);
}
