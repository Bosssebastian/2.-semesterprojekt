#include "Orchestrator.h"
#include "io/IoRunner.h"
#include "web/WebServerRunner.h"
#include "vision/VisionRunner.h"
#include "Types.h"
#include <chrono>
#include <iostream>
#include <cstdio>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#endif

namespace {
constexpr auto kOrchestratorLoopDelay = std::chrono::milliseconds(10);

bool canStopFromState(OrchestratorState state) {
    return state != OrchestratorState::Stopped &&
           state != OrchestratorState::Stopping &&
           state != OrchestratorState::Idle &&
           state != OrchestratorState::Faulted;
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
            if (mState == OrchestratorState::Idle && !command.objectId.empty()) {
                mRequestedObjectId = command.objectId;
                transitionTo(OrchestratorState::InStor_StorageMoveToSlot_Cmd);
            }
            break;
    }
}

void Orchestrator::handleStarting() {
    onStateEnter("Orchestrator: Starting up...\n");
    transitionTo(OrchestratorState::Stopped);
}

void Orchestrator::handleIdle() {
    if (!onStateEnter("Orchestrator: Idle. Waiting for input...\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::InStor_StorageMoveToSlot_Cmd);
}

void Orchestrator::handleInStorStorageMoveToSlotCmd() {
    if (!onStateEnter("Orchestrator: Storage move to slot placeholder state.\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::InStor_RobotOverInput_Cmd);
}

void Orchestrator::handleInStorRobotOverInputCmd() {
    if (!onStateEnter("Orchestrator: Commanding move over input...\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::InStor_RobotOverInput_Wait);
}

void Orchestrator::handleInStorRobotOverInputWait() {
    if (!onStateEnter("Orchestrator: Waiting for move over input to complete...\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::InStor_RobotToCube_Cmd);
}

void Orchestrator::handleInStorRobotToCubeCmd() {
    if (!onStateEnter("Orchestrator: Commanding move to cube...\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::InStor_RobotToCube_Wait);
}

void Orchestrator::handleInStorRobotToCubeWait() {
    if (!onStateEnter("Orchestrator: Waiting for move to cube to complete...\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::InStor_GripperClose_Cmd);
}

void Orchestrator::handleInStorGripperCloseCmd() {
    onStateEnter("Orchestrator: Commanding gripper close...\n");
    mGripper.sendCommand(CmdType::CLOSE);
    transitionTo(OrchestratorState::InStor_GripperClose_Wait);
}

void Orchestrator::handleInStorGripperCloseWait() {
    onStateEnter("Orchestrator: Waiting for gripper close to complete...\n");
    switch (mGripper.getStatus(CmdType::CLOSE)) {
        case CmdStatus::DONE:
            std::printf("Gripper close completed successfully.\n");
            transitionTo(OrchestratorState::InStor_RobotOverStorage_Cmd);
            break;
        case CmdStatus::FAILED:
        case CmdStatus::TIMED_OUT:
            std::printf("Gripper close failed.\n");
            transitionToFault("Gripper failed to close");
            break;
        default:
            break;
    }
}

void Orchestrator::handleInStorRobotOverStorageCmd() {
    if (!onStateEnter("Orchestrator: Robot over storage placeholder state.\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::InStor_RobotOverStorage_Wait);
}

void Orchestrator::handleInStorRobotOverStorageWait() {
    if (!onStateEnter("Orchestrator: Waiting over storage placeholder state.\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::InStor_StorageMoveToPos_Cmd);
}

void Orchestrator::handleInStorStorageMoveToPosCmd() {
    if (!onStateEnter("Orchestrator: Storage move to position placeholder state.\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::InStor_StorageMoveToPos_Wait);
}

void Orchestrator::handleInStorStorageMoveToPosWait() {
    if (!onStateEnter("Orchestrator: Waiting for storage move placeholder state.\n", true)) {
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
    mRequestedObjectId.clear();
    transitionTo(OrchestratorState::Idle);
}

void Orchestrator::handleStopping() {
    onStateEnter("Orchestrator: Stopping system...\n");
    mGripper.sendCommand(CmdType::STOP);
    mStorage.sendCommand(CmdType::STOP);
    mRequestedObjectId.clear();
    transitionTo(OrchestratorState::Stopped);
}


bool Orchestrator::onStateEnter(const char* message, bool waitForEnter) {
    if (mStateJustEntered) {
        std::printf("%s", message);
        if (waitForEnter) {
            std::printf("Press Enter to continue...\n");
        }
        std::fflush(stdout);
        mStateJustEntered = false;
    }

    if (!waitForEnter) {
        return true;
    }

#ifdef _WIN32
    while (_kbhit()) {
        const int ch = _getch();
        if (ch == '\r' || ch == '\n') {
            return true;
        }
    }
    return false;
#else
    std::string line;
    if (!std::getline(std::cin, line)) {
        return false;
    }
    return true;
#endif
}
