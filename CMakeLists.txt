cmake_minimum_required(VERSION 3.16)
project(mcp_server LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(mcp_server
    src/main.cpp
    src/config/config.cpp
    src/logger/logger.cpp
    src/json_rpc/jsonrpc.cpp
    src/json_rpc/jsonrpc_serialization.cpp
    src/json_rpc/stdio_jsonrpc.cpp
)

target_include_directories(mcp_server PRIVATE
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/logger
    ${CMAKE_SOURCE_DIR}/src/json_rpc
)

target_link_libraries(mcp_server PRIVATE spdlog::spdlog nlohmann_json::nlohmann_json)
