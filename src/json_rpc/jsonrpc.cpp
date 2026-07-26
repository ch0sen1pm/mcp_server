#include "jsonrpc.h"

namespace mcp {
void JsonRpcDispatcher::registerHandler(const std::string& method, Handler handler) {
    handlers_[method] = std::move(handler);
}

bool JsonRpcDispatcher::hasHandler(const std::string& method) const {
    return handlers_.find(method) != handlers_.end();
}

json JsonRpcDispatcher::call(const std::string& method, const json& params) const {
    auto it = handlers_.find(method);
    if (it == handlers_.end()) {
        throw std::runtime_error("Method not found " + method);
    }

    return it->second(params);
}

}