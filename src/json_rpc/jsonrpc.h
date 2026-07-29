/**
 * @file jsonrpc.h
 * @brief JSON-RPC 2.0 协议核心 —— 消息类型 + 方法调度器 + Stdio 传输层
 *
 * 架构：
 *   JsonRpcRequest/Response/Error   → 协议消息类型
 *   JsonRpcDispatcher               → method名 → handler 函数 的路由
 *   StdioJsonRpcServer              → Content-Length 封包 + stdin/stdout 读写
 */

#pragma once

#include <nlohmann/json.hpp>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <iostream>

namespace mcp {

using json = nlohmann::json;

// ============================================================
// JSON-RPC 2.0 消息结构体
// ============================================================

/// 错误对象
struct JsonRpcError {
    int code;                        ///< 错误码（-32700 Parse / -32600 Invalid / -32601 NotFound 等）
    std::string message;             ///< 人类可读错误信息
    std::optional<json> data;        ///< 额外错误详情（可选）
};

/// 请求对象
struct JsonRpcRequest {
    std::string jsonrpc = "2.0";     ///< 协议版本，固定 "2.0"
    std::optional<json> id;          ///< 请求ID（无id=通知，不需要响应）
    std::string method;              ///< 方法名 "tools/list" / "tools/call" 等
    std::optional<json> params;      ///< 参数（可选，对象或数组）
};

/// 响应对象
struct JsonRpcResponse {
    std::string jsonrpc = "2.0";     ///< 协议版本
    json id;                         ///< 与请求的 id 对应
    std::optional<json> result;      ///< 成功时的结果（与 error 互斥）
    std::optional<JsonRpcError> error; ///< 失败时的错误（与 result 互斥）
};

// ============================================================
// JSON-RPC 方法调度器
// ============================================================

/// 方法路由器：method名 → handler函数的映射，O(1) 查找
class JsonRpcDispatcher {
public:
    using Handler = std::function<json(const json& params)>;

    void registerHandler(const std::string& method, Handler handler);
    bool hasHandler(const std::string& method) const;
    json call(const std::string& method, const json& params) const; // 抛异常如果 method 不存在

private:
    std::unordered_map<std::string, Handler> handlers_;
};

// ============================================================
// Stdio 传输层 —— Content-Length 封包协议
// ============================================================

/// stdio 传输：从 stdin 读取 Content-Length 封包的 JSON-RPC 请求，写入 stdout
/// 协议格式: "Content-Length: <bytes>\r\n\r\n<JSON body>"
class StdioJsonRpcServer {
public:
    explicit StdioJsonRpcServer(JsonRpcDispatcher dispatcher);
    /// 可指定自定义输入输出流（方便单元测试）
    StdioJsonRpcServer(JsonRpcDispatcher dispatcher, std::istream& in, std::ostream& out);

    void run();  ///< 阻塞主循环

private:
    bool readMessage(std::string& out_body);                   ///< 按 Content-Length 读完整 JSON body
    void writeMessage(const json& msg);                        ///< 写 Content-Length 头 + JSON body
    JsonRpcResponse handleRequest(const JsonRpcRequest& req); ///< 验证 + 分发 + 错误处理

    JsonRpcDispatcher dispatcher_;
    std::istream& in_ = std::cin;    ///< 默认标准输入（可替换为 stringstream 做测试）
    std::ostream& out_ = std::cout;  ///< 默认标准输出
};

// ============================================================
// JSON-RPC 2.0 标准错误码
// ============================================================

namespace jsonrpc_errc {
    constexpr int ParseError     = -32700;  ///< JSON 解析失败
    constexpr int InvalidRequest = -32600;  ///< 请求格式非法
    constexpr int MethodNotFound = -32601;  ///< 方法不存在
    constexpr int InvalidParams  = -32602;  ///< 参数非法
    constexpr int InternalError  = -32603;  ///< 服务器内部错误
}

} // namespace mcp
