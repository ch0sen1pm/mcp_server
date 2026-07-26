#include "config.h"
#include "logger.h"
#include "jsonrpc.h"
#include "jsonrpc_serialization.h"
#include "http_jsonrpc.h"
#include <httplib.h>
#include <iostream>
#include <thread>
#include <chrono>

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

    // 启动 HTTP 服务器（后台线程）
    HttpJsonRpcServer server(dispatcher, "0.0.0.0", 8080);
    std::thread server_thread([&server]() {
        server.run();
    });

    // 等服务器就绪
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 用 httplib::Client 发请求测试
    httplib::Client cli("localhost", 8080);

    auto res = cli.Post("/jsonrpc",
        R"({"jsonrpc":"2.0","method":"echo","id":1,"params":{"msg":"hello http"}})",
        "application/json");

    if (res) {
        std::cout << "HTTP status: " << res->status << std::endl;
        std::cout << "Response: " << res->body << std::endl;
    } else {
        std::cerr << "Request failed!" << std::endl;
    }

    // 再测一条 add
    auto res2 = cli.Post("/jsonrpc",
        R"({"jsonrpc":"2.0","method":"add","id":2,"params":{"a":10,"b":20}})",
        "application/json");
    if (res2) {
        std::cout << "Add result: " << res2->body << std::endl;
    }

    // 健康检查
    auto health = cli.Get("/health");
    if (health) {
        std::cout << "Health: " << health->body << std::endl;
    }

    server.stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }

    MCP_LOG_INFO("HTTP test complete");
    MCP_LOG_FLUSH();
    MCP_LOG_SHUTDOWN();
    return 0;
}
