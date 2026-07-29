/**
 * @file main.cpp
 * @brief MCP Server 入口 —— 注册工具/资源/提示词，启动服务器
 *
 * 支持三种模式：
 *   --mode stdio    stdio 模式（Content-Length 封包，适合 Claude Desktop）
 *   --mode http     HTTP 模式（POST /jsonrpc，适合浏览器/curl）
 *   --mode both     同时启动两种模式
 */

#include "config.h"
#include "logger.h"
#include "jsonrpc.h"
#include "jsonrpc_serialization.h"
#include "http_jsonrpc.h"
#include "mcp_server.h"
#include <iostream>
#include <thread>
#include <csignal>

using namespace mcp;

static std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running = false;
}

// ===== 注册所有 MCP 工具/资源/提示词 =====
void setup_mcp(McpServer& mcp) {
    // ---------- 工具 ----------

    // echo — 最简单工具
    {
        Tool tool;
        tool.name        = "echo";
        tool.description = "Echo back the input message";
        tool.input_schema.properties = {
            {"message", {{"type", "string"}, {"description", "Message to echo"}}}
        };
        tool.input_schema.required = {"message"};

        mcp.register_tool(tool, [](const json& args) -> ToolResult {
            ToolResult r;
            r.content.push_back({"text", "Echo: " + args.at("message").get<std::string>(), {}, {}});
            return r;
        });
    }

    // calculate — 四则运算
    {
        Tool tool;
        tool.name        = "calculate";
        tool.description = "Basic arithmetic: add / subtract / multiply / divide";
        tool.input_schema.properties = {
            {"operation", {{"type", "string"}, {"enum", json::array({"add","subtract","multiply","divide"})}}},
            {"a", {{"type", "number"}}},
            {"b", {{"type", "number"}}}
        };
        tool.input_schema.required = {"operation", "a", "b"};

        mcp.register_tool(tool, [](const json& args) -> ToolResult {
            std::string op = args.at("operation").get<std::string>();
            double a = args.at("a").get<double>();
            double b = args.at("b").get<double>();
            double result = 0;

            if      (op == "add")      result = a + b;
            else if (op == "subtract") result = a - b;
            else if (op == "multiply")    result = a * b;
            else if (op == "divide") {
                if (b == 0) {
                    ToolResult err;
                    err.is_error = true;
                    err.content.push_back({"text", "Error: Division by zero", {}, {}});
                    return err;
                }
                result = a / b;
            }

            ToolResult r;
            r.content.push_back({"text", std::to_string(result), {}, {}});
            return r;
        });
    }

    // get_time — 获取当前时间
    {
        Tool tool;
        tool.name        = "get_time";
        tool.description = "Get current system time";

        mcp.register_tool(tool, [](const json&) -> ToolResult {
            std::time_t now = std::time(nullptr);
            char buf[64];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
            ToolResult r;
            r.content.push_back({"text", buf, {}, {}});
            return r;
        });
    }

    // ---------- 资源 ----------

    // system://info — 系统基础信息
    {
        Resource res;
        res.uri         = "system://info";
        res.name        = "System Information";
        res.description = "Basic system status";
        res.mime_type   = "text/plain";

        mcp.register_resource(res, [](const std::string& uri) -> ResourceContent {
            ResourceContent c;
            c.uri       = uri;
            c.mime_type = "text/plain";
            std::time_t now = std::time(nullptr);
            char buf[64];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
            c.text = std::string("MCP Server Status\n") +
                     "=================\n" +
                     "Time: " + buf + "\n" +
                     "Version: 1.0.0\n";
            return c;
        });
    }

    // ---------- 提示词 ----------

    // code_review — 代码审查模板
    {
        Prompt prompt;
        prompt.name        = "code_review";
        prompt.description = "Generate a code review prompt";
        prompt.arguments   = {
            {"code",     "Code to review", true},
            {"language", "Programming language", true}
        };

        mcp.register_prompt(prompt, [](const json& args) -> std::vector<PromptMessage> {
            PromptMessage msg;
            msg.role = Role::User;
            msg.content = {
                {"type", "text"},
                {"text", "Please review this " + args.at("language").get<std::string>() +
                         " code:\n\n" + args.at("code").get<std::string>()}
            };
            return {msg};
        });
    }

    MCP_LOG_INFO("Setup complete: {} tools, {} resources, {} prompts",
                 mcp.list_tools().size(),
                 mcp.list_resources().size(),
                 mcp.list_prompts().size());
}

// ===== 创建 JSON-RPC 调度器（把 McpServer 的方法注册到 Dispatcher） =====
JsonRpcDispatcher create_dispatcher(McpServer& mcp) {
    JsonRpcDispatcher dispatcher;

    // MCP 协议规定的方法名
    dispatcher.registerHandler("initialize",     [&](const json&)   { return mcp.get_initialize_result().to_json(); });
    dispatcher.registerHandler("tools/list",     [&](const json&)   {
        json arr = json::array();
        for (auto& t : mcp.list_tools()) arr.push_back(t.to_json());
        return json{{"tools", arr}};
    });
    dispatcher.registerHandler("tools/call",     [&](const json& p) {
        return mcp.call_tool(p.at("name").get<std::string>(),
                             p.value("arguments", json::object())).to_json();
    });
    dispatcher.registerHandler("resources/list", [&](const json&)   {
        json arr = json::array();
        for (auto& r : mcp.list_resources()) arr.push_back(r.to_json());
        return json{{"resources", arr}};
    });
    dispatcher.registerHandler("resources/read", [&](const json& p) {
        return mcp.read_resource(p.at("uri").get<std::string>()).to_json();
    });
    dispatcher.registerHandler("prompts/list",   [&](const json&)   {
        json arr = json::array();
        for (auto& pr : mcp.list_prompts()) arr.push_back(pr.to_json());
        return json{{"prompts", arr}};
    });
    dispatcher.registerHandler("prompts/get",    [&](const json& p) {
        auto msgs = mcp.get_prompt(p.at("name").get<std::string>(),
                                   p.value("arguments", json::object()));
        json arr = json::array();
        for (auto& m : msgs) arr.push_back(m.to_json());
        return json{{"messages", arr}};
    });

    return dispatcher;
}

// ===== 主函数 =====
int main(int argc, char* argv[]) {
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    // 解析参数
    std::string mode = "http";
    std::string host = "0.0.0.0";
    int port = 0;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--mode"  && i+1 < argc) mode = argv[++i];
        if (arg == "--host"  && i+1 < argc) host = argv[++i];
        if (arg == "--port"  && i+1 < argc) port = std::stoi(argv[++i]);
    }

    // 配置 + 日志
    MCP_CONFIG.LoadFromFile("../config/server.json");
    MCP_LOG_INIT("mcp_server", "../logs/server.log");
    MCP_LOG_SET_LEVEL(spdlog::level::info);
    if (port == 0) port = MCP_CONFIG.getServerPort();

    // 组装各层
    McpServer mcp("mcp-server", "1.0.0");
    ServerCapabilities caps;
    caps.tools     = ServerCapabilities::ToolsCapability{false};
    caps.resources = ServerCapabilities::ResourcesCapability{false, false};
    caps.prompts   = ServerCapabilities::PromptsCapability{false};
    mcp.set_capabilities(caps);

    setup_mcp(mcp);
    auto dispatcher = create_dispatcher(mcp);

    MCP_LOG_INFO("Starting in {} mode on {}:{}", mode, host, port);

    if (mode == "stdio") {
        StdioJsonRpcServer server(std::move(dispatcher));
        server.run();
    }
    else if (mode == "http") {
        HttpJsonRpcServer server(std::move(dispatcher), host, port);
        server.run();
    }
    else if (mode == "both") {
        HttpJsonRpcServer http(std::move(dispatcher), host, port);
        auto disp_copy = create_dispatcher(mcp);  // Dispatcher 不能共享，拷贝一份
        StdioJsonRpcServer stdio(std::move(disp_copy));

        std::thread http_thread([&http]() { http.run(); });
        stdio.run();
        http.stop();
        if (http_thread.joinable()) http_thread.join();
    }
    else {
        std::cerr << "Unknown mode: " << mode << " (use stdio/http/both)\n";
        return 1;
    }

    MCP_LOG_SHUTDOWN();
    return 0;
}
