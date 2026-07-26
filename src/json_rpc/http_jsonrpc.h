#pragma once

#include "jsonrpc.h"
#include <string>
#include <functional>
#include <memory>
#include <atomic>

namespace mcp {
class HttpJsonRpcServer {
public:
    explicit HttpJsonRpcServer(JsonRpcDispatcher dispatcher);
    HttpJsonRpcServer(JsonRpcDispatcher dispatcher, const std::string& host, int port);

    ~HttpJsonRpcServer();

    HttpJsonRpcServer(const HttpJsonRpcServer&) = delete;
    HttpJsonRpcServer& operator=(const HttpJsonRpcServer&) = delete;

    void run();
    void stop();
    
    bool is_running() const { return running_.load(); }
    const std::string& host() const { return host_; }
    int port() const { return port_; }
    
private:
    JsonRpcDispatcher dispatcher_;
    std::string host_;
    int port_;
    std::atomic<bool> running_{false};

    class Impl;
    std::unique_ptr<Impl> impl_;

    std::string handleRequest(const std::string& request_body);
};
}