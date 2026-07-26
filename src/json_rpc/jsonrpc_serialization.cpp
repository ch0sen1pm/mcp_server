#include "jsonrpc_serialization.h"

namespace mcp {
void to_json(json& j, const JsonRpcError& e) {
    j = json{{"code", e.code}, {"message", e.message}};
    if (e.data.has_value()) {
        j["data"] = *e.data;
    }
} 

void from_json(const json& j, JsonRpcError& e) {
    e.code = j.at("code").get<int>();
    e.message = j.at("message").get<std::string>();

    if (j.contains("data")) {
        e.data = j.at("data");
    } else {
        e.data.reset();
    }
}

void to_json(json& j, const JsonRpcResponse& e) {
    j = json{{"jsonrpc", "2.0"}, {"id", e.id}};
    if (e.result.has_value()) {
        j["result"] = *e.result;
    }
    if (e.error.has_value()) {
        j["error"] = *e.error;
    }
} 

void from_json(const json& j, JsonRpcResponse& e) {
    e.id = j.at("id");
    e.jsonrpc = j.at("jsonrpc").get<std::string>();
    if (j.contains("result")) {
        e.result = j.at("result");
    } else {
        e.result.reset();
    }

    if (j.contains("error")) {
        e.error = j.at("error");
    } else {
        e.error.reset();
    }
}

void to_json(json& j, const JsonRpcRequest& e) {
    j = json{{"jsonrpc", "2.0"}, {"method", e.method}};
    if (e.id.has_value()) {
        j["id"] = *e.id;
    }
    if (e.params.has_value()) {
        j["params"] = *e.params;
    }
}

void from_json(const json& j, JsonRpcRequest& e) {
    e.method = j.at("method").get<std::string>();
    e.jsonrpc = j.at("jsonrpc").get<std::string>();
    if (j.contains("id")) {
        e.id = j.at("id");
    } else {
        e.id.reset();
    }

    if (j.contains("params")) {
        e.params = j.at("params");
    } else {
        e.params.reset();
    }
}

}