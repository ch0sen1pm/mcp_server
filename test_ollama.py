import requests
import json

# Ollama API 默认端口 11434
url = "http://localhost:11434/api/generate"

payload = {
    "model": "qwen2.5:7b",
    "prompt": "用一句话介绍自己",
    "stream": False
}

response = requests.post(url, json=payload)
data = response.json()
print(data["response"])
