#!/usr/bin/env python3
"""Ollama + MCP Server —— 让本地 AI 调用你的 C++ 工具"""

import json, requests, sys

MCP   = "http://localhost:8080/jsonrpc"
MODEL = "qwen2.5:7b"
rid   = 0

def rpc(method, params=None):
    global rid; rid += 1
    r = requests.post(MCP, json={"jsonrpc":"2.0","method":method,"params":params or {},"id":rid})
    return r.json().get("result", {})

# 1. 列出 MCP 工具
tools = rpc("tools/list").get("tools", [])
print(f"获取到 {len(tools)} 个 MCP 工具:")
for t in tools:
    print(f"  - {t['name']}: {t['description']}")

# 2. 转成 Ollama function calling 格式
ollama_tools = []
for t in tools:
    ollama_tools.append({
        "type": "function",
        "function": {
            "name": t["name"],
            "description": t["description"],
            "parameters": {
                "type": "object",
                "properties": t["inputSchema"]["properties"] or {},
                "required": t["inputSchema"].get("required", [])
            }
        }
    })

# 3. 问 AI
query = "现在几点？" if len(sys.argv) < 2 else " ".join(sys.argv[1:])
print(f"\n👤 问: {query}\n")

r = requests.post("http://localhost:11434/api/chat", json={
    "model": MODEL,
    "messages": [{"role":"user","content":query}],
    "tools": ollama_tools,
    "stream": False
}).json()

msg = r.get("message", {})
if "tool_calls" in msg:
    tc = msg["tool_calls"][0]
    name = tc["function"]["name"]
    args = tc["function"]["arguments"]
    print(f"🔧 AI 决定调工具: {name}({json.dumps(args, ensure_ascii=False)})")

    # 调 MCP 工具
    result = rpc("tools/call", {"name": name, "arguments": args})
    text = result["content"][0].get("text", "")
    print(f"   工具返回: {text}")

    # 发回给 AI 总结
    r2 = requests.post("http://localhost:11434/api/chat", json={
        "model": MODEL,
        "messages": [
            {"role":"user","content":query},
            msg,
            {"role":"tool","content":text}
        ],
        "stream": False
    }).json()
    print(f"💬 AI: {r2.get('message',{}).get('content','')}")
else:
    print(f"💬 AI: {msg.get('content','')}")
