/**
 * @file jsonrpc_serialization.h
 * @brief JSON-RPC 消息类型的序列化声明
 *
 * nlohmann::json 通过 ADL（参数依赖查找）找到这些 to_json/from_json 函数
 * 使得可以直接写: json j = request;  或  auto req = j.get<JsonRpcRequest>();
 */

#pragma once

#include "jsonrpc.h"

namespace mcp {

void to_json(json& j, const JsonRpcRequest& r);
void from_json(const json& j, JsonRpcRequest& r);
void to_json(json& j, const JsonRpcError& e);
void from_json(const json& j, JsonRpcError& e);
void to_json(json& j, const JsonRpcResponse& r);
void from_json(const json& j, JsonRpcResponse& r);

} // namespace mcp