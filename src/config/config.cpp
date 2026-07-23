#include "config.h"
#include <fstream>
#include <iostream>

namespace mcp {
Config& Config::GetInstance() {
    static Config instance;
    return instance;
}

bool Config::LoadFromFile(const std::string& path) {
    std::ifstream f(path);

    if (!f.is_open()) {
        std::cerr << "[Config] 无法打开: " << path << std::endl;
        return false;
    }

    try {
        f >> data_;
    } catch(const json::parse_error& e) {
        std::cerr << "[Config] JSON 解析错误: " << e.what() << std::endl;
        return false;
    }

    SetDefaults();

    if (!Validate()) {
        return false;
    }

    loaded_ = true;
    return true;
}

void Config::SetDefaults() {
    if (!data_.contains("server")) {
        data_["server"] = json::object();
    }
    auto& server = data_["server"];

    if (!server.contains("port")) {
        server["port"] = 8080;
    }

    if (!data_.contains("logging")) {
        data_["logging"] = json::object();
    }
    auto& logging = data_["logging"];
    if (!logging.contains("log_level")) {
        logging["log_level"] = "info";
    }
}

bool Config::Validate() const {
    if (!data_.contains("server")) {
        std::cerr << "[Config] 缺少 server 配置段" << std::endl;
        return false;
    }

    int port = data_["server"].value("port", 8080);
    if (port < 1 || port > 65535) {
        std::cerr << "[Config] 端口非法: " << port << std::endl;
        return false;
    }

    if (data_.contains("logging")) {
        std::string level = data_["logging"].value("log_level", "info");
        if (level != "trace" && level != "debug" && level != "info" &&
            level != "warn"  && level != "error" && level != "critical") {
            std::cerr << "[Config] 非法日志级别: " << level << std::endl;
            return false;
        }
    }

    return true;
}

int Config::getServerPort() const {
    return data_["server"].value("port", 8080);
}

std::string Config::GetLogLevel() const {
    return data_["logging"].value("log_level", "info");
}

} // namespace mcp
