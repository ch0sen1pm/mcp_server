/**
 * @file config.h
 * @brief MCP Server 配置管理（单例模式）
 *
 * 功能：从 JSON 文件加载服务器配置，提供默认值，校验合法性
 * 用法：MCP_CONFIG.LoadFromFile("server.json") → MCP_CONFIG.getServerPort()
 */

#pragma once
#include <nlohmann/json.hpp>
#include <string>

namespace mcp {
using json = nlohmann::json;

class Config {
public:
    /// 获取全局唯一实例（Meyer's Singleton，C++11 线程安全）
    static Config& GetInstance();

    /// 从 JSON 文件加载配置，失败返回 false
    bool LoadFromFile(const std::string& path);

    /// 服务器端口（默认 8080）
    int getServerPort() const;

    /// 日志级别字符串（trace/debug/info/warn/error/critical）
    std::string GetLogLevel() const;

private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;            // 单例禁止拷贝
    Config& operator=(const Config&) = delete;

    bool Validate() const;                     // 校验端口范围、日志级别等
    void SetDefaults();                        // 补全缺失字段

    json data_;
    bool loaded_ = false;
};

/// 便捷宏：全局唯一 Config 引用
#define MCP_CONFIG Config::GetInstance()

} // namespace mcp
