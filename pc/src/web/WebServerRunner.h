#pragma once

#include "orchestrator/OrchestratorState.h"
#include <atomic>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

enum class WebCommandType {
    Start,
    Stop,
    GetObject
};

struct WebCommand {
    WebCommandType type{WebCommandType::Start};
    std::string objectId;
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

    std::thread mWorker;
    std::atomic<bool> mRunning{false};
    mutable std::mutex mCommandMutex;
    std::optional<WebCommand> mPendingCommand;
    mutable std::mutex mStateMutex;
    OrchestratorState mState{OrchestratorState::Stopped};
};
