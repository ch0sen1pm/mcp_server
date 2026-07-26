#pragma once

#include <nlohmann/json.hpp>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

namespace mcp {

using json = nlohmann::json;

struct JsonRpcError {
    int code;
    std::string message;
    std::optional<json> data;
};

struct JsonRpcRequest {
    std::string jsonrpc = "2.0";
    std::optional<json> id;
    std::string method;
    std::optional<json> params;
};

struct JsonRpcResponse {
    std::string jsonrpc = "2.0";
    json id;
    std::optional<json> result;
    std::optional<JsonRpcError> error;
};

class JsonRpcDispatcher {
public:
    using Handler = std::function<json(const json& params)>;

    void registerHandler(const std::string& method, Handler handler);

    bool hasHandler(const std::string& method) const;

    json call(const std::string& method, const json& params) const;
private:
    std::unordered_map<std::string, Handler> handlers_;
};

namespace jsonrpc_errc {
    constexpr int ParseError     = -32700;
    constexpr int InvalidRequest = -32600;
    constexpr int MethodNotFound = -32601;
    constexpr int InvalidParams  = -32602;
    constexpr int InternalError  = -32603;
}

}