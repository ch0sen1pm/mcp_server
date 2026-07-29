/**
 * @file mcp_server.h
 * @brief MCP 协议核心 —— Tools / Resources / Prompts 三原语管理
 *
 * 设计：
 *   每个原语维护两个 map（元信息 + handler）和一个 mutex：
 *     tools_ + tool_handlers_          → 工具（AI 可调用）
 *     resources_ + resource_providers_ → 资源（AI 可读取的只读数据）
 *     prompts_ + prompt_generators_    → 提示词（AI 可请求的对话模板）
 *
 *   所有读写操作加锁（std::lock_guard），支持多线程并发访问
 */

#pragma once

#include "types.h"
#include <functional>
#include <unordered_map>
#include <mutex>

namespace mcp {

class McpServer {
public:
    // --- 三个 Handler 类型别名 ---
    using ToolHandler      = std::function<ToolResult(const json& arguments)>;
    using ResourceProvider = std::function<ResourceContent(const std::string& uri)>;
    using PromptGenerator  = std::function<std::vector<PromptMessage>(const json& arguments)>;

    McpServer(const std::string& name, const std::string& version);

    /// 返回 initialize 握手响应（协议版本 + 能力 + 服务器信息）
    InitializeResult get_initialize_result() const;
    void set_capabilities(const ServerCapabilities& capabilities);

    // ===== Tool（工具）=====
    void register_tool(const Tool& tool, ToolHandler handler);         ///< 注册工具（重复抛异常）
    std::vector<Tool> list_tools() const;                              ///< 列出所有工具
    ToolResult call_tool(const std::string& name, const json& args);   ///< 调用工具
    bool has_tool(const std::string& name) const;                      ///< 检查是否存在

    // ===== Resource（资源）=====
    void register_resource(const Resource& r, ResourceProvider p);
    std::vector<Resource> list_resources() const;
    ResourceContent read_resource(const std::string& uri);
    bool has_resource(const std::string& uri) const;

    // ===== Prompt（提示词）=====
    void register_prompt(const Prompt& prompt, PromptGenerator gen);
    std::vector<Prompt> list_prompts() const;
    std::vector<PromptMessage> get_prompt(const std::string& name, const json& args);
    bool has_prompt(const std::string& name) const;

    // SSE 事件回调（预留，用于 HTTP SSE 推送工具调用状态）
    using SseEventCallback = std::function<void(const json&)>;
    void set_sse_callback(SseEventCallback callback);

private:
    ServerInfo server_info_;
    ServerCapabilities capabilities_;

    // --- Tool ---
    std::unordered_map<std::string, Tool> tools_;              ///< 元信息（list_tools 用）
    std::unordered_map<std::string, ToolHandler> tool_handlers_;//< 实际函数（call_tool 用）
    mutable std::mutex tools_mutex_;                           ///< mutable = const 函数也能上锁

    // --- Resource ---
    std::unordered_map<std::string, Resource> resources_;
    std::unordered_map<std::string, ResourceProvider> resource_providers_;
    mutable std::mutex resources_mutex_;

    // --- Prompt ---
    std::unordered_map<std::string, Prompt> prompts_;
    std::unordered_map<std::string, PromptGenerator> prompt_generators_;
    mutable std::mutex prompts_mutex_;

    // --- SSE ---
    SseEventCallback sse_callback_;
    mutable std::mutex sse_mutex_;
};

} // namespace mcp
