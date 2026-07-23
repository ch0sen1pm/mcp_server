cmake_minimum_required(VERSION 3.16)
project(mcp_server LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(mcp_server
    src/main.cpp
    src/config/config.cpp
)

target_link_libraries(mcp_server PRIVATE spdlog::spdlog nlohmann_json::nlohmann_json)
