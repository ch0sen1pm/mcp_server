#include "config.h"
#include "logger.h"
#include "jsonrpc.h"
#include "jsonrpc_serialization.h"
#include <iostream>
#include <sstream>

using namespace mcp;

int main() {
    MCP_CONFIG.LoadFromFile("../config/server.json");
    MCP_LOG_INIT("mcp_server", "../logs/server.log");
    MCP_LOG_SET_LEVEL(spdlog::level::debug);

    // 准备 Dispatcher
    JsonRpcDispatcher dispatcher;
    dispatcher.registerHandler("echo", [](const json& params) -> json {
        return {{"you_sent", params}};
    });
    dispatcher.registerHandler("add", [](const json& params) -> json {
        return params.at("a").get<int>() + params.at("b").get<int>();
    });

    // 构造假的 stdin（用 json::dump() 生成，Content-Length 自动算对）
    auto make_msg = [](const json& j) -> std::string {
        std::string body = j.dump();
        return "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
    };

    std::string fake_input =
        make_msg({{"jsonrpc", "2.0"}, {"method", "echo"}, {"id", 1}, {"params", {{"msg", "hi"}}}}) +
        make_msg({{"jsonrpc", "2.0"}, {"method", "add"}, {"id", 2}, {"params", {{"a", 3}, {"b", 4}}}});

    std::istringstream fake_in(fake_input);
    std::ostringstream fake_out;

    // 用自定义流运行 stdio server
    StdioJsonRpcServer server(dispatcher, fake_in, fake_out);
    server.run();

    // 打印 fake stdout 的输出
    std::cout << "=== Server output ===\n" << fake_out.str() << std::endl;

    MCP_LOG_INFO("Stdio test complete");
    MCP_LOG_FLUSH();
    MCP_LOG_SHUTDOWN();
    return 0;
}
