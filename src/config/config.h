#pragma once
#include <nlohmann/json.hpp>
#include <string>

namespace mcp {
using json = nlohmann::json;

class Config {
public:
    static Config& GetInstance();

    bool LoadFromFile(const std::string& path);

    int getServerPort() const;

    std::string GetLogLevel() const;

private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    bool Validate() const;
    void SetDefaults();

    json data_;
    bool loaded_ = false;
};

#define MCP_CONFIG Config::GetInstance()

} // namespace mcp