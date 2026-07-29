/**
 * @file http_jsonrpc.h
 * @brief HTTP 传输层 —— 使用 cpp-httplib 提供 POST /jsonrpc 端点
 *
 * 设计：使用 Pimpl（Pointer to Implementation）惯用法将 httplib 头文件隐藏在 .cpp 中
 *       避免 httplib 的大量代码污染调用方的编译单元
 *       class Impl 仅前向声明，实际定义在 http_jsonrpc.cpp
 */

#pragma once

#include "jsonrpc.h"
#include <string>
#include <functional>
#include <memory>
#include <atomic>

namespace mcp {

class HttpJsonRpcServer {
public:
    /// 使用默认地址 0.0.0.0:8080
    explicit HttpJsonRpcServer(JsonRpcDispatcher dispatcher);
    /// 自定义 host + port
    HttpJsonRpcServer(JsonRpcDispatcher dispatcher, const std::string& host, int port);

    ~HttpJsonRpcServer();

    // 禁止拷贝（持有 unique_ptr，天然不可拷贝，显式声明更清晰）
    HttpJsonRpcServer(const HttpJsonRpcServer&) = delete;
    HttpJsonRpcServer& operator=(const HttpJsonRpcServer&) = delete;

    void run();   ///< 阻塞：启动 HTTP 服务器
    void stop();  ///< 线程安全停止（通过 atomic<bool> 跨线程通信）

    bool is_running() const { return running_.load(); }  ///< atomic 读取，线程安全
    const std::string& host() const { return host_; }
    int port() const { return port_; }

private:
    JsonRpcDispatcher dispatcher_;
    std::string host_;
    int port_;
    std::atomic<bool> running_{false};  ///< atomic：run/stop 跨线程可见

    class Impl;                         ///< Pimpl：前向声明，不暴露 httplib
    std::unique_ptr<Impl> impl_;        ///< 独占所有权

    std::string handleRequest(const std::string& request_body); ///< 解析 + 分发 + 返回响应
};

} // namespace mcp
