#include "config/config.h"
#include "logger/logger.h"
#include <iostream>

using namespace mcp;

int main() {
    // 1. 加载配置
    if (!MCP_CONFIG.LoadFromFile("../config/server.json")) {
        std::cerr << "Failed to load config" << std::endl;
        return 1;
    }

    // 2. 初始化日志
    MCP_LOG_INIT("mcp_server", "../logs/server.log");

    // 用配置里的 log_level（需要把 "debug" 转为 spdlog::level::debug）
    MCP_LOG_SET_LEVEL(spdlog::level::debug);

    // 3. 测试各级别日志
    MCP_LOG_INFO("Server starting...");
    MCP_LOG_DEBUG("Server port: {}", MCP_CONFIG.getServerPort());
    MCP_LOG_WARN("This is a warning test");
    MCP_LOG_ERROR("This is an error test");

    MCP_LOG_INFO("Config check — port: {}, log_level: {}",
                 MCP_CONFIG.getServerPort(),
                 MCP_CONFIG.GetLogLevel());

    // 4. 刷盘 + 关闭
    MCP_LOG_FLUSH();
    MCP_LOG_SHUTDOWN();
    return 0;
}
