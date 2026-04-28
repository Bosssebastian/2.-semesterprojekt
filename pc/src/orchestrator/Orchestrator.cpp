#include "Orchestrator.h"
#include "io/IoRunner.h"
#include "logging/Logger.h"
#include "web/WebServerRunner.h"
#include "vision/VisionRunner.h"
#include "Types.h"
#include <chrono>
#include <cstdio>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#else
#include <sys/select.h>
#include <unistd.h>
#endif

namespace {
constexpr auto kOrchestratorLoopDelay = std::chrono::milliseconds(10);

bool canStopFromState(OrchestratorState state) {
    return state != OrchestratorState::Stopped &&
           state != OrchestratorState::Resetting &&
           state != OrchestratorState::Stopping &&
           state != OrchestratorState::Faulted;
}

bool pollTerminalEnter() {
#ifdef _WIN32
    bool sawEnter = false;
    while (_kbhit()) {
        const int ch = _getch();
        if (ch == '\r' || ch == '\n') {
            sawEnter = true;
        }
    }
    return sawEnter;
#else
    bool sawEnter = false;
    while (true) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(STDIN_FILENO, &readSet);

        timeval timeout{};
        const int ready = select(STDIN_FILENO + 1, &readSet, nullptr, nullptr, &timeout);
        if (ready <= 0 || !FD_ISSET(STDIN_FILENO, &readSet)) {
            break;
        }

        char buffer[256];
        const ssize_t bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            break;
        }

        for (ssize_t index = 0; index < bytesRead; ++index) {
            if (buffer[index] == '\r' || buffer[index] == '\n') {
                sawEnter = true;
            }
        }
    }

    return sawEnter;
#endif
}
}

Orchestrator::Orchestrator(IoRunner& io, VisionRunner& vision, WebServerRunner& web)
    : mIo(io), mVision(vision), mWeb(web), mGripper(io.gripper()), mStorage(io.storage()) {
    mWeb.setState(mState);
}

void Orchestrator::run() {
    start();

    while (!mStopRequested) {
        update();
        std::this_thread::sleep_for(kOrchestratorLoopDelay);
    }
}

void Orchestrator::start() {
    mStopRequested = false;
    mFaultReason.clear();
    transitionTo(OrchestratorState::Stopped);
}


void Orchestrator::update() {
    if (mWeb.hasCommand()) {
        handleWebCommand(mWeb.getCommand());
    }

    switch (mState) {
        case OrchestratorState::Starting: handleStarting(); break;
        case OrchestratorState::Idle: handleIdle(); break;

        case OrchestratorState::Resetting: handleResetting(); break;
        case OrchestratorState::Stopping: handleStopping(); break;
        case OrchestratorState::Stopped: break;
        case OrchestratorState::Faulted: break;


        // Input to storage cycle states
        case OrchestratorState::InStor_StorageMoveToSlot: handleInStorStorageMoveToSlot(); break;
        case OrchestratorState::InStor_RobotOverInput: handleInStorRobotOverInput(); break;
        case OrchestratorState::InStor_RobotToCube: handleInStorRobotToCube(); break;
        case OrchestratorState::InStor_GripperClose: handleInStorGripperClose(); break;
        case OrchestratorState::InStor_RobotOverStorage: handleInStorRobotOverStorage(); break;
        case OrchestratorState::InStor_StorageMoveToPos: handleInStorStorageMoveToPos(); break;
        case OrchestratorState::InStor_RobotDownToSlot: handleInStorRobotDownToSlot(); break;
        case OrchestratorState::InStor_GripperOpen: handleInStorGripperOpen(); break;
        case OrchestratorState::InStor_RobotUpFromSlot: handleInStorRobotUpFromSlot(); break;

        case OrchestratorState::InStor_Complete: handleInStorComplete(); break;
    }
}

OrchestratorState Orchestrator::getState() const {
    return mState;
}

const std::string& Orchestrator::getFaultReason() const {
    return mFaultReason;
}

void Orchestrator::transitionTo(OrchestratorState newState) {
    mState = newState;
    mStateJustEntered = true;
    mWeb.setState(mState);
}

void Orchestrator::transitionToFault(const std::string& reason) {
    LOG_ERROR(reason);
    mFaultReason = reason;
    stopMotion();
    transitionTo(OrchestratorState::Faulted);
}

void Orchestrator::handleWebCommand(const WebCommand& command) {
    switch (command.type) {
        case WebCommandType::Start:
            if (mState == OrchestratorState::Stopped) {
                LOG_INFO("Start command received");
                transitionTo(OrchestratorState::Idle);
            }
            break;
        case WebCommandType::Stop:
            if (canStopFromState(mState)) {
                LOG_INFO("Stop command received");
                transitionTo(OrchestratorState::Stopping);
            }
            break;
        case WebCommandType::Reset:
            if (mState == OrchestratorState::Faulted) {
                LOG_INFO("Reset command received");
                transitionTo(OrchestratorState::Resetting);
            }
            break;
        case WebCommandType::SkipReq:
            mPendingSkipRequest = true;
            break;
    }
}

bool Orchestrator::skipRequested() {
    if (mPendingSkipRequest) {
        mPendingSkipRequest = false;
        return true;
    }

    return pollTerminalEnter();
}

void Orchestrator::handleStarting() {
    onEnter([] {
        LOG_INFO("Orchestrator: Starting up...");
    });
    transitionTo(OrchestratorState::Stopped);
}

void Orchestrator::handleIdle() {
    onEnter([] {
        LOG_INFO("Orchestrator: Idle. Waiting for input...");
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_StorageMoveToSlot);
        return;
    }
}

void Orchestrator::handleInStorStorageMoveToSlot() {
    onEnter([] {
        LOG_INFO("Orchestrator: Storage move to slot placeholder state.");
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotOverInput);
        return;
    }
}

void Orchestrator::handleInStorRobotOverInput() {
    onEnter([] {
        LOG_INFO("Orchestrator: Commanding move over input...");
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotToCube);
        return;
    }
}

void Orchestrator::handleInStorRobotToCube() {
    onEnter([] {
        LOG_INFO("Orchestrator: Commanding move to cube...");
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_GripperClose);
        return;
    }
}

void Orchestrator::handleInStorGripperClose() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Commanding gripper close...");
        mGripper.sendCommand(CmdType::CLOSE);
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotOverStorage);
        return;
    }

    switch (mGripper.getStatus(CmdType::CLOSE)) {
        case CmdStatus::DONE:
            LOG_INFO("Gripper close completed successfully.");
            transitionTo(OrchestratorState::InStor_RobotOverStorage);
            break;
        case CmdStatus::FAILED:
            transitionToFault("Gripper failed to close");
            break;
        case CmdStatus::TIMED_OUT:
            transitionToFault("Gripper close timed out");
            break;
        default:
            break;
    }
}

void Orchestrator::handleInStorRobotOverStorage() {
    onEnter([] {
        LOG_INFO("Orchestrator: Robot over storage placeholder state.");
    });

    if (!skipRequested()) {
        return;
    }
    transitionTo(OrchestratorState::InStor_StorageMoveToPos);
}

void Orchestrator::handleInStorStorageMoveToPos() {
    onEnter([] {
        LOG_INFO("Orchestrator: Storage move to position placeholder state.");
    });

    if (!skipRequested()) {
        return;
    }
    transitionTo(OrchestratorState::InStor_RobotDownToSlot);
}

void Orchestrator::handleInStorRobotDownToSlot() {
    onEnter([] {
        LOG_INFO("Orchestrator: Robot down to slot placeholder state.");
    });

    if (!skipRequested()) {
        return;
    }
    transitionTo(OrchestratorState::InStor_GripperOpen);
}

void Orchestrator::handleInStorGripperOpen() {
    onEnter([] {
        LOG_INFO("Orchestrator: Gripper open placeholder state.");
    });

    if (!skipRequested()) {
        return;
    }
    transitionTo(OrchestratorState::InStor_RobotUpFromSlot);
}

void Orchestrator::handleInStorRobotUpFromSlot() {
    onEnter([] {
        LOG_INFO("Orchestrator: Robot up from slot placeholder state.");
    });

    if (!skipRequested()) {
        return;
    }
    transitionTo(OrchestratorState::InStor_Complete);
}

void Orchestrator::handleInStorComplete() {
    onEnter([] {
        LOG_INFO("Orchestrator: Input-to-storage cycle complete.");
    });
    transitionTo(OrchestratorState::Idle);
}

void Orchestrator::handleResetting() {
    onEnter([] {
        LOG_INFO("Orchestrator: Resetting system...");
    });
    stopMotion();
    mFaultReason.clear();
    mPendingSkipRequest = false;
    transitionTo(OrchestratorState::Stopped);
}

void Orchestrator::handleStopping() {
    onEnter([] {
        LOG_INFO("Orchestrator: Stopping system...");
    });
    stopMotion();
    transitionTo(OrchestratorState::Stopped);
}

void Orchestrator::stopMotion() {
    mGripper.sendCommand(CmdType::STOP);
    mStorage.sendCommand(CmdType::STOP);
    //Robot stop
}
