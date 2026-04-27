#pragma once

#include "orchestrator/OrchestratorState.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace httplib {
class Request;
class Response;
class Server;
}

enum class WebCommandType {
    Start,
    Stop,
    Reset,
    SkipReq
};

struct WebCommand {
    WebCommandType type{WebCommandType::Start};
};

class WebServerRunner {
public:
    WebServerRunner();
    ~WebServerRunner();

    void start();
    void stop();

    bool hasCommand() const;
    WebCommand getCommand();

    void setState(OrchestratorState state);
    OrchestratorState getState() const;

private:
    void run();

    bool tryStoreCommand(const WebCommand& command);
    bool queueCommand(WebCommandType type);

    void handleGetState(const httplib::Request& request, httplib::Response& response) const;
    void handleGetLogs(const httplib::Request& request, httplib::Response& response) const;
    void handleStart(const httplib::Request& request, httplib::Response& response);
    void handleStop(const httplib::Request& request, httplib::Response& response);
    void handleReset(const httplib::Request& request, httplib::Response& response);
    void handleSkipReq(const httplib::Request& request, httplib::Response& response);

    std::thread mWorker;
    std::atomic<bool> mRunning{false};
    mutable std::mutex mServerMutex;
    std::unique_ptr<httplib::Server> mServer;
    mutable std::mutex mCommandMutex;
    std::optional<WebCommand> mPendingCommand;
    mutable std::mutex mStateMutex;
    OrchestratorState mState{OrchestratorState::Stopped};
};
