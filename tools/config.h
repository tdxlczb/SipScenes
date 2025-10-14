#pragma once
#include <string>
#include <map>


class Config
{
public:
    bool Load(const std::string& filename);

    std::string GetString(const std::string& section, const std::string& key,
        const std::string& default_value = "");

    int GetInt(const std::string& section, const std::string& key, int default_value = 0);

    double GetDouble(const std::string& section, const std::string& key, double default_value = 0.0);

    bool GetBool(const std::string& section, const std::string& key, bool default_value = false);

    // 显示所有配置（用于调试）
    void Display();

private:
    // 去除字符串两端的空白字符
    std::string trim(const std::string& str);
private:
    std::map<std::string, std::map<std::string, std::string>> m_data;
};