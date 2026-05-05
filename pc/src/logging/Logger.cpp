#include "logging/Logger.h"

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

namespace {
std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp) {
    const std::time_t rawTime = std::chrono::system_clock::to_time_t(timestamp);
    std::tm timeInfo{};

#ifdef _WIN32
    localtime_s(&timeInfo, &rawTime);
#else
    localtime_r(&rawTime, &timeInfo);
#endif

    std::ostringstream stream;
    stream << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    return stream.str();
}
}

const char* toString(LogType type) {
    switch (type) {
        case LogType::Info:
            return "INFO";
        case LogType::Warn:
            return "WARN";
        case LogType::Err:
            return "ERR";
    }

    return "INFO";
}

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::info(std::string message) {
    log(LogType::Info, std::move(message));
}

void Logger::warn(std::string message) {
    log(LogType::Warn, std::move(message));
}

void Logger::error(std::string message) {
    log(LogType::Err, std::move(message));
}

void Logger::log(LogType type, std::string message) {
    LogEntry entry{std::chrono::system_clock::now(), type, std::move(message)};

    {
        std::lock_guard<std::mutex> lock(mMutex);
        mEntries.push_back(entry);
        trimToCapacityLocked();
    }

    const std::string timestamp = formatTimestamp(entry.timestamp);
    std::printf("[%s] [%s] %s\n", timestamp.c_str(), toString(entry.type), entry.message.c_str());
    std::fflush(stdout);
}

std::vector<LogEntry> Logger::entries() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return std::vector<LogEntry>(mEntries.begin(), mEntries.end());
}

void Logger::clear() {
    std::lock_guard<std::mutex> lock(mMutex);
    mEntries.clear();
}

void Logger::setMaxEntries(std::size_t maxEntries) {
    std::lock_guard<std::mutex> lock(mMutex);
    mMaxEntries = maxEntries;
    trimToCapacityLocked();
}

std::size_t Logger::maxEntries() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mMaxEntries;
}

void Logger::trimToCapacityLocked() {
    while (mEntries.size() > mMaxEntries) {
        mEntries.pop_front();
    }
}
