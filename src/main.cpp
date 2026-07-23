#include "config/config.h"
#include <iostream>

using namespace mcp;

int main() {
    if (!MCP_CONFIG.LoadFromFile("../config/server.json")) {
        std::cerr << "Failed to load config" << std::endl;
        return 1;
    }

    std::cout << "Port: " << MCP_CONFIG.getServerPort() << std::endl;
    std::cout << "Log level: " << MCP_CONFIG.GetLogLevel() << std::endl;
    std::cout << "Config OK!" << std::endl;
    return 0;
}
