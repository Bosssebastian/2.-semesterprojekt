#include "WebServerRunner.h"

#include "httplib.h"
#include "logging/Logger.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {
constexpr int kServerPort = 8081;
constexpr const char* kServerHost = "127.0.0.1";

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());

    for (const char c : value) {
        switch (c) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += c;
                break;
        }
    }

    return escaped;
}

std::string formatLogTimestampForApi(const std::chrono::system_clock::time_point& timestamp) {
    const std::time_t rawTime = std::chrono::system_clock::to_time_t(timestamp);
    std::tm timeInfo{};

#ifdef _WIN32
    localtime_s(&timeInfo, &rawTime);
#else
    localtime_r(&rawTime, &timeInfo);
#endif

    std::ostringstream stream;
    stream << std::put_time(&timeInfo, "%H:%M:%S");
    return stream.str();
}

std::string buildLogsJsonBody() {
    const auto entries = Logger::instance().entries();

    std::ostringstream stream;
    stream << "{\"entries\":[";

    for (std::size_t index = 0; index < entries.size(); ++index) {
        const auto& entry = entries[index];
        if (index > 0) {
            stream << ',';
        }

        stream << "{\"timestamp\":\"" << jsonEscape(formatLogTimestampForApi(entry.timestamp))
               << "\",\"type\":\"" << jsonEscape(toString(entry.type))
               << "\",\"message\":\"" << jsonEscape(entry.message) << "\"}";
    }

    stream << "]}";
    return stream.str();
}

std::string buildStateJsonBody(OrchestratorState state) {
    std::ostringstream stream;
    stream << "{\"state\":\"" << jsonEscape(toString(state))
           << "\",\"stateLabel\":\"" << jsonEscape(toUiString(state)) << "\"}";
    return stream.str();
}

}

WebServerRunner::WebServerRunner() = default;

WebServerRunner::~WebServerRunner() {
    stop();
}

void WebServerRunner::start() {
    if (mRunning.exchange(true)) {
        return;
    }

    mWorker = std::thread(&WebServerRunner::run, this);
}

void WebServerRunner::stop() {
    mRunning = false;

    {
        std::lock_guard<std::mutex> lock(mServerMutex);
        if (mServer) {
            mServer->stop();
        }
    }

    if (mWorker.joinable()) {
        mWorker.join();
    }
}

bool WebServerRunner::hasCommand() const {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    return mPendingCommand.has_value();
}

WebCommand WebServerRunner::getCommand() {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    if (!mPendingCommand.has_value()) {
        throw std::runtime_error("No pending web command");
    }

    WebCommand command = *mPendingCommand;
    mPendingCommand.reset();
    return command;
}

void WebServerRunner::setState(OrchestratorState state) {
    std::lock_guard<std::mutex> lock(mStateMutex);
    mState = state;
}

OrchestratorState WebServerRunner::getState() const {
    std::lock_guard<std::mutex> lock(mStateMutex);
    return mState;
}

bool WebServerRunner::tryStoreCommand(const WebCommand& command) {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    if (mPendingCommand.has_value()) {
        return false;
    }

    mPendingCommand = command;
    return true;
}

bool WebServerRunner::queueCommand(WebCommandType type) {
    return tryStoreCommand(WebCommand{type});
}

void WebServerRunner::handleGetState(const httplib::Request&, httplib::Response& response) const {
    response.set_content(buildStateJsonBody(getState()), "application/json");
    response.set_header("Access-Control-Allow-Origin", "*");
}

void WebServerRunner::handleGetLogs(const httplib::Request&, httplib::Response& response) const {
    response.set_content(buildLogsJsonBody(), "application/json");
    response.set_header("Access-Control-Allow-Origin", "*");
}

void WebServerRunner::handleStart(const httplib::Request&, httplib::Response& response) {
    if (!queueCommand(WebCommandType::Start)) {
        response.status = 409;
        return;
    }

    response.status = 204;
}

void WebServerRunner::handleStop(const httplib::Request&, httplib::Response& response) {
    if (!queueCommand(WebCommandType::Stop)) {
        response.status = 409;
        return;
    }

    response.status = 204;
}

void WebServerRunner::handleReset(const httplib::Request&, httplib::Response& response) {
    if (!queueCommand(WebCommandType::Reset)) {
        response.status = 409;
        return;
    }

    response.status = 204;
}

void WebServerRunner::handleSkipReq(const httplib::Request&, httplib::Response& response) {
    if (!queueCommand(WebCommandType::SkipReq)) {
        response.status = 409;
        return;
    }

    response.status = 204;
}

void WebServerRunner::run() {
#ifdef _WIN32
    LOG_WARN("WebServerRunner Windows support is unverified with cpp-httplib");
#endif

    auto server = std::make_unique<httplib::Server>();
    server->Get("/getstate", [this](const httplib::Request& request, httplib::Response& response) {
        handleGetState(request, response);
    });
    server->Get("/logs", [this](const httplib::Request& request, httplib::Response& response) {
        handleGetLogs(request, response);
    });
    server->Post("/cmdStart", [this](const httplib::Request& request, httplib::Response& response) {
        handleStart(request, response);
    });
    server->Post("/cmdStop", [this](const httplib::Request& request, httplib::Response& response) {
        handleStop(request, response);
    });
    server->Post("/cmdReset", [this](const httplib::Request& request, httplib::Response& response) {
        handleReset(request, response);
    });
    server->Post("/cmdSkipReq", [this](const httplib::Request& request, httplib::Response& response) {
        handleSkipReq(request, response);
    });

    {
        std::lock_guard<std::mutex> lock(mServerMutex);
        mServer = std::move(server);
    }

    LOG_INFO("Web server is started");

    bool listenOk = false;
    {
        std::lock_guard<std::mutex> lock(mServerMutex);
        if (mServer) {
            listenOk = mServer->listen(kServerHost, kServerPort);
        }
    }

    if (!listenOk && mRunning) {
        LOG_ERROR("WebServerRunner failed to listen on 127.0.0.1:8081");
    }

    {
        std::lock_guard<std::mutex> lock(mServerMutex);
        mServer.reset();
    }

    mRunning = false;

}
