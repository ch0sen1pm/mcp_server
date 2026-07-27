#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace mcp {
using json = nlohmann::json;

constexpr const char* LATEST_PROTOCOL_VERSION = "2024-11-5";

struct ToolInputSchema {
    std::string type = "object";
    json properties;
    std::vector<std::string> required;

    json to_json() const;
    static ToolInputSchema from_json(const json& j);
};

struct Tool {
    std::string name;
    std::string description;
    ToolInputSchema input_schema;

    json to_json() const;
    static Tool from_json(const json& j);
};

struct ContentItem {
    std::string type;
    std::optional<std::string> text;
    std::optional<std::string> data;
    std::optional<std::string> mime_type;

    json to_json() const;
    static ContentItem from_json(const json& j);
};

struct ToolResult {
    std::vector<ContentItem> content;
    bool is_error = false;

    json to_json() const;
    static ToolResult from_json(const json& j);
};

struct Resource {
    std::string uri;
    std::string name;
    std::optional<std::string> description;
    std::optional<std::string> mime_type;

    json to_json() const;
    static Resource from_json(const json& j);
};

struct ResourceContent {
    std::string uri;
    std::optional<std::string> mime_type;
    std::string text;
    std::optional<std::string> blob;

    json to_json() const;
    static ResourceContent from_json(const json& j);
};

enum class Role { User, Assistant };

struct PromptArgument {
    std::string name;
    std::optional<std::string> description;
    bool required = false;

    json to_json() const;
    static PromptArgument from_json(const json& j);
};

struct Prompt {
    std::string name;
    std::optional<std::string> description;
    std::vector<PromptArgument> arguments;

    json to_json() const;
    static Prompt from_json(const json& j);
};

struct PromptMessage {
    Role role;
    json content;

    json to_json() const;
    static PromptMessage from_json(const json& j);
};

struct ServerInfo {
    std::string name;
    std::string version;

    json to_json() const;
    static ServerInfo from_json(const json& j);
};

struct ServerCapabilities {
    struct ToolsCapability {
        bool list_changed = false;
        json to_json() const;
        static ToolsCapability from_json(const json& j);
    };

    struct ResourcesCapability {
        bool subscribe = false;
        bool list_changed = false;
        json to_json() const;
        static ResourcesCapability from_json(const json& j);
    };

    struct PromptsCapability {
        bool list_changed = false;
        json to_json() const;
        static PromptsCapability from_json(const json& j);
    };

    std::optional<ToolsCapability> tools;
    std::optional<ResourcesCapability> resources;
    std::optional<PromptsCapability> prompts;
    std::optional<json> logging;

    json to_json() const;
    static ServerCapabilities from_json(const json& j);
};

struct InitializeResult {
    std::string protocol_version;
    ServerCapabilities capabilities;
    ServerInfo server_info;

    json to_json() const;
    static InitializeResult from_json(const json& j);
};

}