#include "config.h"
#include "logger.h"
#include "jsonrpc.h"
#include "jsonrpc_serialization.h"
#include <iostream>

using namespace mcp;

int main() {
    // 1. 配置 + 日志
    MCP_CONFIG.LoadFromFile("../config/server.json");
    MCP_LOG_INIT("mcp_server", "../logs/server.log");
    MCP_LOG_SET_LEVEL(spdlog::level::debug);

    // 2. 测试 Dispatcher
    JsonRpcDispatcher dispatcher;

    dispatcher.registerHandler("echo", [](const json& params) -> json {
        return {{"you_sent", params}};
    });

    json result = dispatcher.call("echo", {{"msg", "hello json-rpc"}});
    std::cout << "echo result: " << result.dump(2) << std::endl;

    // 测试方法不存在
    try {
        dispatcher.call("no_such_method", json::object());
    } catch (const std::exception& e) {
        std::cerr << "Expected error: " << e.what() << std::endl;
    }

    MCP_LOG_INFO("Dispatcher test passed");

    // 3. 测试序列化：C++ → JSON → C++
    JsonRpcRequest req;
    req.method = "tools/list";
    req.id = 1;
    req.params = json::object();

    json req_json = req;                               // to_json
    auto req2 = req_json.get<JsonRpcRequest>();        // from_json

    std::cout << "Round-trip: method=" << req2.method
              << " id=" << req2.id->dump() << std::endl;

    MCP_LOG_INFO("Serialization test passed");

    MCP_LOG_FLUSH();
    MCP_LOG_SHUTDOWN();
    return 0;
}
