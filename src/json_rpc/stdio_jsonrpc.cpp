#include "jsonrpc.h"
#include "jsonrpc_serialization.h"
#include "logger.h"

#include <iostream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace mcp {
StdioJsonRpcServer::StdioJsonRpcServer(JsonRpcDispatcher dispatcher)
    : dispatcher_(std::move(dispatcher)) {}

StdioJsonRpcServer::StdioJsonRpcServer(JsonRpcDispatcher dispatcher,
                                       std::istream& in, std::ostream& out)
    : dispatcher_(std::move(dispatcher)), in_(in), out_(out) {}

bool StdioJsonRpcServer::readMessage(std::string& out_body) {
    out_body.clear();

    std::string line;
    size_t content_length = 0;
    bool found = false;

    while (std::getline(in_, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            break;
        }

        auto colon = line.find(":");
        if (colon == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, colon);
        std::string val = line.substr(colon + 1);

        if (!val.empty() && val.front() == ' ') {
            val.erase(0, val.find_first_not_of(' '));
        }

        for (auto& c : key) {
            c = std::tolower(static_cast<unsigned char>(c));
        }

        if (key == "content-length") {
            content_length = std::stoul(val);
            found = true;
        }
    }

    if (!found) {
        return false;
    }
    if (content_length == 0) {
        return true;
    }

    out_body.resize(content_length);
    size_t read = 0;
    while (read < content_length) {
        in_.read(&out_body[read], content_length - read);
        auto n = in_.gcount();
        if (n <= 0) {
            break;
        }
        read += static_cast<size_t>(n);
    }

    return read == content_length;
}

void StdioJsonRpcServer::writeMessage(const json& msg) {
    std::string payload = msg.dump();
    out_ << "Content-Length: " << payload.size() << "\r\n\r\n";
    out_ << payload;
    out_.flush();
}

JsonRpcResponse StdioJsonRpcServer::handleRequest(const JsonRpcRequest& req) {
    JsonRpcResponse resp;
    resp.jsonrpc = "2.0";
    resp.id = req.id.has_value() ? *req.id : json(nullptr);

    try {
        if (req.jsonrpc != "2.0") {
            throw std::invalid_argument("jsonrpc must be 2.0");
        }
        if (req.method.empty()) {
            throw std::invalid_argument("method is empty");
        }

        if (!dispatcher_.hasHandler(req.method)) {
            resp.error = JsonRpcError {
                jsonrpc_errc::MethodNotFound, "Method not found", std::nullopt
                };
            return resp;
        }
        json params = req.params.has_value() ? * req.params : json::object();
        resp.result = dispatcher_.call(req.method, params);
        resp.error.reset();
    }
    catch (const std::invalid_argument& e) {
        resp.error = JsonRpcError{jsonrpc_errc::InvalidRequest, e.what(), std::nullopt};
        resp.result.reset();
    }
    catch (const std::exception& e) {
        resp.error = JsonRpcError{jsonrpc_errc::InternalError, e.what(), std::nullopt};
        resp.result.reset();
    }

    return resp;
}

void StdioJsonRpcServer::run() {
    MCP_LOG_INFO("JSON-RPC stdio server starting...");

    std::string body;
    while (true) {
        if (!readMessage(body)) {
            if (in_.eof()) {
                break;
            }
            continue;
        }

        try {
            json j = json::parse(body);
            JsonRpcRequest req = j.get<JsonRpcRequest>();
            bool is_notification = !req.id.has_value();
            auto resp = handleRequest(req);

            if (!is_notification) {
                writeMessage(json(resp));
            }
        }
        catch (const json::parse_error& e) {
            JsonRpcResponse resp;
            resp.id = nullptr;
            resp.error = JsonRpcError{jsonrpc_errc::ParseError, e.what(), std::nullopt};
            writeMessage(json(resp));
        }

    }
}
}