#include "http_jsonrpc.h"
#include "jsonrpc_serialization.h"
#include "logger.h"

#include <httplib.h>

namespace mcp {
class HttpJsonRpcServer::Impl {
public:
    httplib::Server server;
};

HttpJsonRpcServer::HttpJsonRpcServer(JsonRpcDispatcher dispatcher)
    : HttpJsonRpcServer(std::move(dispatcher), "0.0.0.0", 8080) {}

HttpJsonRpcServer::HttpJsonRpcServer(
    JsonRpcDispatcher dispatcher,
    const std::string& host,
    int port)
    : dispatcher_(std::move(dispatcher))
    , host_(host)
    , port_(port)
    , impl_(std::make_unique<Impl>())
{
    impl_->server.Post("/jsonrpc", [this](const httplib::Request& req,
                                        httplib::Response& res){
        res.set_header("Access-Control-Allow-Origin", "*");

        try {
            std::string response = handleRequest(req.body);
            res.set_content(response, "application/json");
            res.status = 200;
        } catch (const std::exception& e) {
            json err = {{"jsonrpc", "2.0"},
                        {"error", {{"code", -32603}, {"message", e.what()}}},
                        {"id", nullptr}};
            
            res.set_content(err.dump(), "application/json");
            res.status = 500;
        }
    });

    
    impl_->server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json h{{"status", "ok"}};
        res.set_content(h.dump(), "application/json");
    });
}

HttpJsonRpcServer::~HttpJsonRpcServer() {
    stop();
}

void HttpJsonRpcServer::run() {
    running_ = true;
    impl_->server.listen(host_, port_);
    running_ = false;
}

void HttpJsonRpcServer::stop() {
    if (running_) {
        impl_->server.stop();
        running_ = false;
    }
}

std::string HttpJsonRpcServer::handleRequest(const std::string& request_body) {
    json j = json::parse(request_body);
    JsonRpcRequest req = j.get<JsonRpcRequest>();
    JsonRpcResponse resp = req.id.has_value()
          ? JsonRpcResponse{"2.0", *req.id, std::nullopt, std::nullopt}
          : JsonRpcResponse{"2.0", nullptr, std::nullopt, std::nullopt};

    try {
        if (dispatcher_.hasHandler(req.method)) {
            resp.result = dispatcher_.call(req.method,
                req.params.has_value() ? *req.params : json::object());
        } else {
            resp.error = JsonRpcError{jsonrpc_errc::MethodNotFound,
                "Method not found", std::nullopt};
        }
    } catch (const std::exception& e) {
        resp.error = JsonRpcError{jsonrpc_errc::InternalError, e.what(), std::nullopt};
        resp.result.reset();
    }

    json out = {{"jsonrpc", "2.0"}, {"id", resp.id}};
    if (resp.result.has_value()) out["result"] = *resp.result;
    if (resp.error.has_value()) out["error"] = json(*resp.error);

    return out.dump();
}

}