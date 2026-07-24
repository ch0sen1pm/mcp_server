#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>

namespace mcp {
namespace logger {

class Logger {
public:
    static Logger& getInstance();

    void init(const std::string& logger_name = "mcp",
            const std::string& log_file_path = "",
            size_t max_file_size = 10 * 1024 * 1024,
            size_t max_files = 5,
            bool console_output = true);
    
    void setLevel(spdlog::level::level_enum level);
    std::shared_ptr<spdlog::logger> getLogger();
    void flush();
    void shutdown();


private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::shared_ptr<spdlog::logger> m_logger;
    bool m_initialized = false;
};
} // namespace logger
} // namespace mcp

#define MCP_LOG_INIT(name, ...) \
    mcp::logger::Logger::getInstance().init(name, ##__VA_ARGS__)

#define MCP_LOG_SET_LEVEL(level) \
    mcp::logger::Logger::getInstance().setLevel(level)

#define MCP_LOG_FLUSH() \
    mcp::logger::Logger::getInstance().flush()

#define MCP_LOG_SHUTDOWN() \
    mcp::logger::Logger::getInstance().shutdown()

#define MCP_LOG_TRACE(fmt, ...) \
    do { \
        auto logger = mcp::logger::Logger::getInstance().getLogger(); \
        if (logger) logger->trace("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while(0)

#define MCP_LOG_DEBUG(fmt, ...) \
      do { \
          auto logger = mcp::logger::Logger::getInstance().getLogger(); \
          if (logger) logger->debug("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
      } while(0)

#define MCP_LOG_INFO(fmt, ...) \
    do { \
        auto logger = mcp::logger::Logger::getInstance().getLogger(); \
        if (logger) logger->info("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while(0)

#define MCP_LOG_WARN(fmt, ...) \
    do { \
        auto logger = mcp::logger::Logger::getInstance().getLogger(); \
        if (logger) logger->warn("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while(0)

#define MCP_LOG_ERROR(fmt, ...) \
    do { \
        auto logger = mcp::logger::Logger::getInstance().getLogger(); \
        if (logger) logger->error("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while(0)

#define MCP_LOG_CRITICAL(fmt, ...) \
    do { \
        auto logger = mcp::logger::Logger::getInstance().getLogger(); \
        if (logger) logger->critical("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while(0)
