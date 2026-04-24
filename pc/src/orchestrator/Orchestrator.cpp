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
    mRequestedObjectId.clear();
    transitionTo(OrchestratorState::Stopped);
}


void Orchestrator::update() {
    if (mWeb.hasCommand()) {
        handleWebCommand(mWeb.getCommand());
    }

    switch (mState) {
        case OrchestratorState::Starting: handleStarting(); break;
        case OrchestratorState::Idle: handleIdle(); break;

        case OrchestratorState::Stopping: handleStopping(); break;
        case OrchestratorState::Stopped: break;
        case OrchestratorState::Faulted: break;


        // Input to storage cycle states
        case OrchestratorState::InStor_StorageMoveToSlot_Cmd: handleInStorStorageMoveToSlotCmd(); break;

        case OrchestratorState::InStor_RobotOverInput_Cmd: handleInStorRobotOverInputCmd(); break;
        case OrchestratorState::InStor_RobotOverInput_Wait: handleInStorRobotOverInputWait(); break;

        case OrchestratorState::InStor_RobotToCube_Cmd: handleInStorRobotToCubeCmd(); break;
        case OrchestratorState::InStor_RobotToCube_Wait: handleInStorRobotToCubeWait(); break;

        case OrchestratorState::InStor_GripperClose_Cmd: handleInStorGripperCloseCmd(); break;
        case OrchestratorState::InStor_GripperClose_Wait: handleInStorGripperCloseWait(); break;

        case OrchestratorState::InStor_RobotOverStorage_Cmd: handleInStorRobotOverStorageCmd(); break;
        case OrchestratorState::InStor_RobotOverStorage_Wait: handleInStorRobotOverStorageWait(); break;

        case OrchestratorState::InStor_StorageMoveToPos_Cmd: handleInStorStorageMoveToPosCmd(); break;
        case OrchestratorState::InStor_StorageMoveToPos_Wait: handleInStorStorageMoveToPosWait(); break;

        case OrchestratorState::InStor_RobotDownToSlot_Cmd: handleInStorRobotDownToSlotCmd(); break;
        case OrchestratorState::InStor_RobotDownToSlot_Wait: handleInStorRobotDownToSlotWait(); break;

        case OrchestratorState::InStor_GripperOpen_Cmd: handleInStorGripperOpenCmd(); break;
        case OrchestratorState::InStor_GripperOpen_Wait: handleInStorGripperOpenWait(); break;

        case OrchestratorState::InStor_RobotUpFromSlot_Cmd: handleInStorRobotUpFromSlotCmd(); break;
        case OrchestratorState::InStor_RobotUpFromSlot_Wait: handleInStorRobotUpFromSlotWait(); break;

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
    transitionTo(OrchestratorState::Faulted);
}

void Orchestrator::handleWebCommand(const WebCommand& command) {
    switch (command.type) {
        case WebCommandType::Start:
            if (mState == OrchestratorState::Stopped) {
                transitionTo(OrchestratorState::Idle);
            }
            break;
        case WebCommandType::Stop:
            if (canStopFromState(mState)) {
                transitionTo(OrchestratorState::Stopping);
            }
            break;
        case WebCommandType::GetObject:
            //TODO
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
    onStateEnter("Orchestrator: Starting up...\n");
    transitionTo(OrchestratorState::Stopped);
}

void Orchestrator::handleIdle() {
    onStateEnter("Orchestrator: Idle. Waiting for input...\n");
    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_StorageMoveToSlot_Cmd);
        return;
    }
}

void Orchestrator::handleInStorStorageMoveToSlotCmd() {
    onStateEnter("Orchestrator: Storage move to slot placeholder state.\n");
    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotOverInput_Cmd);
        return;
    }
}

void Orchestrator::handleInStorRobotOverInputCmd() {
    onStateEnter("Orchestrator: Commanding move over input...\n");
    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotOverInput_Wait);
        return;
    }
}

void Orchestrator::handleInStorRobotOverInputWait() {
    onStateEnter("Orchestrator: Waiting for move over input to complete...\n");
    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotToCube_Cmd);
        return;
    }
}

void Orchestrator::handleInStorRobotToCubeCmd() {
    onStateEnter("Orchestrator: Commanding move to cube...\n");
    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotToCube_Wait);
        return;
    }
}

void Orchestrator::handleInStorRobotToCubeWait() {
    onStateEnter("Orchestrator: Waiting for move to cube to complete...\n");
    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_GripperClose_Cmd);
        return;
    }
}

void Orchestrator::handleInStorGripperCloseCmd() {
    onStateEnter("Orchestrator: Commanding gripper close...\n");
    mGripper.sendCommand(CmdType::CLOSE);
    transitionTo(OrchestratorState::InStor_GripperClose_Wait);
}

void Orchestrator::handleInStorGripperCloseWait() {
    onStateEnter("Orchestrator: Waiting for gripper close to complete...\n");

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotOverStorage_Cmd);
        return;
    }

    switch (mGripper.getStatus(CmdType::CLOSE)) {
        case CmdStatus::DONE:
            LOG_INFO("Gripper close completed successfully.");
            transitionTo(OrchestratorState::InStor_RobotOverStorage_Cmd);
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

void Orchestrator::handleInStorRobotOverStorageCmd() {
    onStateEnter("Orchestrator: Robot over storage placeholder state.\n");
    if (!skipRequested()) {
        return;
    }
    transitionTo(OrchestratorState::InStor_RobotOverStorage_Wait);
}

void Orchestrator::handleInStorRobotOverStorageWait() {
    onStateEnter("Orchestrator: Waiting over storage placeholder state.\n");
    if (!skipRequested()) {
        return;
    }
    transitionTo(OrchestratorState::InStor_StorageMoveToPos_Cmd);
}

void Orchestrator::handleInStorStorageMoveToPosCmd() {
    onStateEnter("Orchestrator: Storage move to position placeholder state.\n");
    if (!skipRequested()) {
        return;
    }
    transitionTo(OrchestratorState::InStor_StorageMoveToPos_Wait);
}

void Orchestrator::handleInStorStorageMoveToPosWait() {
    onStateEnter("Orchestrator: Waiting for storage move placeholder state.\n");
    if (!skipRequested()) {
        return;
    }
    transitionTo(OrchestratorState::InStor_RobotDownToSlot_Cmd);
}

void Orchestrator::handleInStorRobotDownToSlotCmd() {
    onStateEnter("Orchestrator: Robot down to slot placeholder state.\n");
}

void Orchestrator::handleInStorRobotDownToSlotWait() {
    onStateEnter("Orchestrator: Waiting for robot down to slot placeholder state.\n");
}

void Orchestrator::handleInStorGripperOpenCmd() {
    onStateEnter("Orchestrator: Gripper open placeholder state.\n");
}

void Orchestrator::handleInStorGripperOpenWait() {
    onStateEnter("Orchestrator: Waiting for gripper open placeholder state.\n");
}

void Orchestrator::handleInStorRobotUpFromSlotCmd() {
    onStateEnter("Orchestrator: Robot up from slot placeholder state.\n");
}

void Orchestrator::handleInStorRobotUpFromSlotWait() {
    onStateEnter("Orchestrator: Waiting for robot up from slot placeholder state.\n");
}

void Orchestrator::handleInStorComplete() {
    onStateEnter("Orchestrator: Input-to-storage cycle complete.\n");
    transitionTo(OrchestratorState::Idle);
}

void Orchestrator::handleStopping() {
    onStateEnter("Orchestrator: Stopping system...\n");
    mGripper.sendCommand(CmdType::STOP);
    mStorage.sendCommand(CmdType::STOP);
    //Robot stop
    transitionTo(OrchestratorState::Stopped);
}


void Orchestrator::onStateEnter(const char* message) {
    if (mStateJustEntered) {
        LOG_INFO(message);
        mStateJustEntered = false;
    }
}
