# ============================================================
# MCP Server — C++17 CMake 构建文件
#
# 依赖：spdlog（日志）、nlohmann-json（JSON 序列化）、cpp-httplib（HTTP header-only）
# 包管理：vcpkg
# ============================================================

cmake_minimum_required(VERSION 3.16)
project(mcp_server LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# --- 依赖（通过 vcpkg）---
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# --- 所有源文件 ---
add_executable(mcp_server
    src/main.cpp                      # 入口：注册工具 + 启动服务器
    src/config/config.cpp             # 配置管理（单例）
    src/logger/logger.cpp             # 日志系统（spdlog 封装）
    src/json_rpc/jsonrpc.cpp          # 方法调度器实现
    src/json_rpc/jsonrpc_serialization.cpp # C++ ↔ JSON 序列化
    src/json_rpc/stdio_jsonrpc.cpp    # stdio 传输层（Content-Length）
    src/json_rpc/http_jsonrpc.cpp     # HTTP 传输层（cpp-httplib + Pimpl）
    src/mcp/types.cpp                 # MCP 协议数据结构
    src/mcp/mcp_server.cpp            # MCP 核心（Tools/Resources/Prompts 管理）
)

# --- 包含路径（按模块划分，方便 #include "xxx.h" 直接引用）---
target_include_directories(mcp_server PRIVATE
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/logger
    ${CMAKE_SOURCE_DIR}/src/json_rpc
    ${CMAKE_SOURCE_DIR}/src/mcp
    /mnt/d/vcpkg-linux/installed/x64-linux/include  # vcpkg 安装的 httplib.h
)

# --- 链接库 ---
target_link_libraries(mcp_server PRIVATE
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)
