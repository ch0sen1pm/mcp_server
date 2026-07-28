#include "types.h"


namespace mcp {
json ToolInputSchema::to_json() const {
    json j = {{"type", type}, {"properties", properties}};

    if (!required.empty()) {
        j["required"] = required;
    }

    return j;
}

ToolInputSchema ToolInputSchema::from_json(const json& j) {
    ToolInputSchema s;
    s.type       = j.value("type", "object");
    s.properties = j.value("properties", json::object());

    if (j.contains("required")) {
        s.required =j["required"].get<std::vector<std::string>>();
    }

    return s;
}

json Tool::to_json() const {
    json j = {{"name", name}, {"description", description}, {"inputSchema", input_schema.to_json()}};

    return j;
}

Tool Tool::from_json(const json& j) {
    Tool s;
    s.name         = j.at("name").get<std::string>();
    s.description  = j.value("description", "");
    s.input_schema = ToolInputSchema::from_json(j.at("inputSchema"));

    return s;
}

json ContentItem::to_json() const {
    json j = {{"type", type}};

    if (text.has_value())      j["text"]     = *text;
    if (data.has_value())      j["data"]     = *data;
    if (mime_type.has_value()) j["mimeType"] = *mime_type;

    return j;
}

ContentItem ContentItem::from_json(const json& j) {
    ContentItem item;

    item.type = j.at("type").get<std::string>();
    if (j.contains("text"))     item.text     = j["text"].get<std::string>();
    if (j.contains("data"))     item.data     = j["data"].get<std::string>();
    if (j.contains("mimeType")) item.mime_type = j["mimeType"].get<std::string>();

    return item;
}

json ToolResult::to_json() const {
    json arr = json::array();
    for (const auto& item : content) {
        arr.push_back(item.to_json());
    }
    json j = {{"content", arr}};
    if (is_error) j["isError"] = true;

    return j;
}

ToolResult ToolResult::from_json(const json& j) {
    ToolResult r;
    if (j.contains("content")) {
        for (const auto& item : j["content"]) {
            r.content.push_back(ContentItem::from_json(item));
        }
    }
    r.is_error = j.value("isError", false);

    return r;
} 

json Resource::to_json() const {
    json j = {{"uri", uri}, {"name", name}};
    if (description.has_value()) j["description"] = *description;
    if (mime_type.has_value())   j["mimeType"]    = *mime_type;

    return j;
}

Resource Resource::from_json(const json& j) {
    Resource r;
    r.uri  = j.at("uri").get<std::string>();
    r.name = j.at("name").get<std::string>();
    if (j.contains("description")) r.description = j["description"].get<std::string>();
    if (j.contains("mimeType"))    r.mime_type   = j["mimeType"].get<std::string>();
    return r;
}

// ===== ResourceContent =====

json ResourceContent::to_json() const {
    json j = {{"uri", uri}, {"text", text}};
    if (mime_type.has_value()) j["mimeType"] = *mime_type;
    if (blob.has_value())      j["blob"]     = *blob;
    return j;
}

ResourceContent ResourceContent::from_json(const json& j) {
    ResourceContent c;
    c.uri  = j.at("uri").get<std::string>();
    c.text = j.value("text", "");
    if (j.contains("mimeType")) c.mime_type = j["mimeType"].get<std::string>();
    if (j.contains("blob"))     c.blob      = j["blob"].get<std::string>();
    return c;
}

// ===== PromptArgument =====

json PromptArgument::to_json() const {
    json j = {{"name", name}, {"required", required}};
    if (description.has_value()) j["description"] = *description;
    return j;
}

PromptArgument PromptArgument::from_json(const json& j) {
    PromptArgument a;
    a.name     = j.at("name").get<std::string>();
    a.required = j.value("required", false);
    if (j.contains("description")) a.description = j["description"].get<std::string>();
    return a;
}

// ===== Prompt =====

json Prompt::to_json() const {
    json j = {{"name", name}};
    if (description.has_value()) j["description"] = *description;
    if (!arguments.empty()) {
        json arr = json::array();
        for (const auto& arg : arguments) arr.push_back(arg.to_json());
        j["arguments"] = arr;
    }
    return j;
}

Prompt Prompt::from_json(const json& j) {
    Prompt p;
    p.name = j.at("name").get<std::string>();
    if (j.contains("description")) p.description = j["description"].get<std::string>();
    if (j.contains("arguments")) {
        for (const auto& a : j["arguments"])
            p.arguments.push_back(PromptArgument::from_json(a));
    }
    return p;
}

// ===== PromptMessage =====

json PromptMessage::to_json() const {
    return {
        {"role", role == Role::User ? "user" : "assistant"},
        {"content", content}
    };
}

PromptMessage PromptMessage::from_json(const json& j) {
    PromptMessage msg;
    std::string s = j.at("role").get<std::string>();
    msg.role    = (s == "user") ? Role::User : Role::Assistant;
    msg.content = j.at("content");
    return msg;
}

// ===== ServerInfo =====

json ServerInfo::to_json() const {
    return {{"name", name}, {"version", version}};
}

ServerInfo ServerInfo::from_json(const json& j) {
    ServerInfo info;
    info.name    = j.at("name").get<std::string>();
    info.version = j.at("version").get<std::string>();
    return info;
}

// ===== ServerCapabilities (含三个嵌套) =====

json ServerCapabilities::ToolsCapability::to_json() const {
    return {{"listChanged", list_changed}};
}
ServerCapabilities::ToolsCapability
ServerCapabilities::ToolsCapability::from_json(const json& j) {
    ToolsCapability c;
    c.list_changed = j.value("listChanged", false);
    return c;
}

json ServerCapabilities::ResourcesCapability::to_json() const {
    return {{"subscribe", subscribe}, {"listChanged", list_changed}};
}
ServerCapabilities::ResourcesCapability
ServerCapabilities::ResourcesCapability::from_json(const json& j) {
    ResourcesCapability c;
    c.subscribe    = j.value("subscribe", false);
    c.list_changed = j.value("listChanged", false);
    return c;
}

json ServerCapabilities::PromptsCapability::to_json() const {
    return {{"listChanged", list_changed}};
}
ServerCapabilities::PromptsCapability
ServerCapabilities::PromptsCapability::from_json(const json& j) {
    PromptsCapability c;
    c.list_changed = j.value("listChanged", false);
    return c;
}

json ServerCapabilities::to_json() const {
    json j = json::object();
    if (tools.has_value())     j["tools"]     = tools->to_json();
    if (resources.has_value()) j["resources"] = resources->to_json();
    if (prompts.has_value())   j["prompts"]   = prompts->to_json();
    if (logging.has_value())   j["logging"]   = *logging;
    return j;
}

ServerCapabilities ServerCapabilities::from_json(const json& j) {
    ServerCapabilities cap;
    if (j.contains("tools"))     cap.tools     = ToolsCapability::from_json(j["tools"]);
    if (j.contains("resources")) cap.resources = ResourcesCapability::from_json(j["resources"]);
    if (j.contains("prompts"))   cap.prompts   = PromptsCapability::from_json(j["prompts"]);
    if (j.contains("logging"))   cap.logging   = j["logging"];
    return cap;
}

// ===== InitializeResult =====

json InitializeResult::to_json() const {
    return {
        {"protocolVersion", protocol_version},
        {"capabilities", capabilities.to_json()},
        {"serverInfo", server_info.to_json()}
    };
}

InitializeResult InitializeResult::from_json(const json& j) {
    InitializeResult r;
    r.protocol_version = j.at("protocolVersion").get<std::string>();
    r.capabilities     = ServerCapabilities::from_json(j.at("capabilities"));
    r.server_info      = ServerInfo::from_json(j.at("serverInfo"));
    return r;
}

} // namespace mcp