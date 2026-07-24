#include "logger.h"
#include <filesystem>
#include <iostream>
#include <vector>

namespace mcp {
namespace logger {
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& logger_name,
                const std::string& log_file_path,
                size_t max_file_size,
                size_t max_files,
                bool console_output) {
    if (m_initialized) {
        return ;
    }    
    try {
        std::vector<spdlog::sink_ptr> sinks;

        if (console_output) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::trace);
            console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] [%n] %v");
            sinks.push_back(console_sink);
        }

        if (!log_file_path.empty()) {
            std::filesystem::path log_path(log_file_path);
            auto log_dir = log_path.parent_path();
            if (!log_dir.empty() && !std::filesystem::exists(log_dir)) {
                std::filesystem::create_directories(log_dir);
            }

            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                log_file_path, max_file_size, max_files);
            
            file_sink->set_level(spdlog::level::trace);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] [%t] %v");
            sinks.push_back(file_sink);
        }
        m_logger = std::make_shared<spdlog::logger>(logger_name, sinks.begin(), sinks.end());
        m_logger->set_level(spdlog::level::info);
        m_logger->flush_on(spdlog::level::info);
        spdlog::register_logger(m_logger);
        spdlog::set_default_logger(m_logger);

        m_initialized = true;
        m_logger->info("Logger initialized: name={}, file={}",
                        logger_name, log_file_path.empty() ? "none" : log_file_path);
    } catch(const spdlog::spdlog_ex& ex) {
        std::cerr << "Logger init failed: " << ex.what() << std::endl;
        throw;
    }
}

void Logger::setLevel(spdlog::level::level_enum level) {
    if (m_logger) {
        m_logger->set_level(level);
    }
}

std::shared_ptr<spdlog::logger> Logger::getLogger() {
    if (!m_initialized) {
        init();
    }
    return m_logger;
}

void Logger::flush() {
    if (m_logger) {
        m_logger->flush();
    }
}

void Logger::shutdown() {
    if (!m_initialized) {
        return;
    }
    try {
        if (m_logger) {
            m_logger->flush();
        }
    } catch (...) {};
    spdlog::shutdown();
    m_logger.reset();
    m_initialized = false;
}

Logger::~Logger() {
    shutdown();
}

}
}