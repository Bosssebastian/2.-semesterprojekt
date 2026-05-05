#pragma once

#include <chrono>
#include <cstddef>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

enum class LogType {
    Info,
    Warn,
    Err
};

const char* toString(LogType type);

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogType type;
    std::string message;
};

class Logger {
public:
    static Logger& instance();

    void info(std::string message);
    void warn(std::string message);
    void error(std::string message);
    void log(LogType type, std::string message);

    std::vector<LogEntry> entries() const;
    void clear();

    void setMaxEntries(std::size_t maxEntries);
    std::size_t maxEntries() const;

private:
    Logger() = default;

    void trimToCapacityLocked();

    mutable std::mutex mMutex;
    std::deque<LogEntry> mEntries;
    std::size_t mMaxEntries{1000};
};

#ifndef LOG_INFO
#define LOG_INFO(message) Logger::instance().info(message)
#endif

#ifndef LOG_WARN
#define LOG_WARN(message) Logger::instance().warn(message)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(message) Logger::instance().error(message)
#endif
