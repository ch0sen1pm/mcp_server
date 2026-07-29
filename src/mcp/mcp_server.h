#pragma once

#include "types.h"
#include <functional>
#include <unordered_map>
#include <mutex>

namespace mcp {
class McpServer {
public:
    using ToolHandler      = std::function<ToolResult(const json& arguments)>;
    using ResourceProvider = std::function<ResourceContent(const std::string& uri)>;
    using PromptGenerator  = std::function<std::vector<PromptMessage>(const json& arguments)>;

    McpServer(const std::string& name, const std::string& version);

    InitializeResult get_initialize_result() const;
    void set_capabilities(const ServerCapabilities& capabilities);

    void register_tool(const Tool& tool, ToolHandler handler);
    std::vector<Tool> list_tools() const;
    ToolResult call_tool(const std::string& name, const json& arguments);
    bool has_tool(const std::string& name) const;

    void register_resource(const Resource& resource, ResourceProvider provider);
    std::vector<Resource> list_resources() const;
    ResourceContent read_resource(const std::string& uri);
    bool has_resource(const std::string& uri) const;

    void register_prompt(const Prompt& prompt, PromptGenerator generator);
    std::vector<Prompt> list_prompts() const;
    std::vector<PromptMessage> get_prompt(const std::string& name, const json& arguments);
    bool has_prompt(const std::string& name) const;

    using SseEventCallback = std::function<void(const json&)>;
    void set_sse_callback(SseEventCallback callback);

private:
    ServerInfo server_info_;
    ServerCapabilities capabilities_;

    std::unordered_map<std::string, Tool> tools_;
    std::unordered_map<std::string, ToolHandler> tool_handlers_;
    mutable std::mutex tools_mutex_;

    std::unordered_map<std::string, Resource> resources_;
    std::unordered_map<std::string, ResourceProvider> resource_providers_;
    mutable std::mutex resources_mutex_;

    std::unordered_map<std::string, Prompt> prompts_;
    std::unordered_map<std::string, PromptGenerator> prompt_generators_;
    mutable std::mutex prompts_mutex_;

    SseEventCallback see_callback_;
    mutable std::mutex sse_mutex_;
};
}