# mcp-server

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

C++ 实现的 MCP（Model Context Protocol）Server。基于 JSON-RPC 协议，支持 Tools / Resources / Prompts 三类原语的注册与调用。AI 客户端通过 stdio 或 HTTP 与 Server 通信，Server 调用本地工具执行命令、查询数据、访问文件。

## Architecture

```
AI 客户端 (Claude / Cursor / Ollama)
  → MCP 协议（JSON-RPC over stdio / HTTP）
  → JsonRpcDispatcher（方法路由：tools/list / tools/call / resources/read ...）
  → McpServer（工具/资源/提示词 注册 + 调用）
  → 具体 Handler（echo / calculate / get_weather / write_file / ask_ai ...）
```

### 模块分层

| 层 | 文件 | 职责 |
|------|------|------|
| **类型系统** | `types.h` | MCP 协议核心数据结构（Tool / Resource / Prompt / ToolResult） |
| **配置** | `config.h` | 单例模式，JSON 文件加载 + 校验 + 默认值 |
| **日志** | `logger.h` | 单例模式，spdlog 封装（双 sink：控制台 + 文件轮转），日志宏自动带文件名行号 |
| **JSON-RPC** | `jsonrpc.h` / `jsonrpc_serialization.h` | JSON-RPC 2.0 消息类型（Request/Response/Error）+ 方法调度器 + 序列化 |
| **传输** | `stdio_jsonrpc.cpp` / `http_jsonrpc.cpp` | Content-Length 封包（stdio）+ HTTP POST /jsonrpc + SSE（Pimpl 封装 cpp-httplib） |
| **MCP 核心** | `mcp_server.h` | 三个 map 管理 Tools / Resources / Prompts 的注册与调用 |
| **主程序** | `main.cpp` | 组装各层 + 注册具体工具 |

## Roadmap

- [x] Config 模块（单例 + JSON 加载 + 校验 + 默认值）
- [x] Logger 模块（spdlog 封装 + 日志宏 + 双 sink）
- [ ] Types 模块（Tool / Resource / Prompt / ToolResult 数据结构）
- [x] JsonRpc 模块（消息类型 + 方法调度器 + nlohmann::json 序列化）
- [ ] McpServer 核心（register_tool / call_tool / list_tools）
- [x] Stdio 传输（Content-Length 封包 + stdin/stdout + 请求分发）
- [ ] 首批工具（echo / calculate / get_time / ask_ai）
- [x] HTTP 传输（POST /jsonrpc + CORS + 健康检查 + Pimpl）
## Quick Start

```bash
# 依赖安装（vcpkg）
vcpkg install nlohmann-json spdlog cpp-httplib

# 编译
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg.cmake
cmake --build build

# 运行
./build/mcp_server
```

## 配置文件

```json
{
    "server": {
        "port": 8089
    },
    "logging": {
        "log_level": "debug"
    }
}
```

## License

MIT
