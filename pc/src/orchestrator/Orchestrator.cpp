#include "Orchestrator.h"
#include "io/IoRunner.h"
#include "vision/VisionRunner.h"
#include "Types.h"
#include <iostream>
#ifdef _WIN32
#include <conio.h>
#endif


Orchestrator::Orchestrator(IoRunner& io, VisionRunner& vision)
    : mIo(io), mVision(vision), mGripper(io.gripper()), mStorage(io.storage()) {
}

void Orchestrator::run() {
    start();

    while (!mStopRequested) {
        update();

        // Main loop
        // Here all classes should have their update functions


    }
}

void Orchestrator::start() {
    mStopRequested = false;
    mFaultReason.clear();

    // Start up
    // Here all classes should have their start functions called

    transitionTo(OrchestratorState::Idle);
}


void Orchestrator::update() {
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
}

void Orchestrator::transitionToFault(const std::string& reason) {
    mFaultReason = reason;
    transitionTo(OrchestratorState::Faulted);
}

void Orchestrator::handleStarting() {
    if (!onStateEnter("Orchestrator: Starting up...\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::Idle);
}

void Orchestrator::handleIdle() {
    if (!onStateEnter("Orchestrator: Idle. Waiting for cube on input...\n", true)) {
        return;
    }
    transitionTo(OrchestratorState::InStor_StorageMoveToSlot_Cmd);
}

void Orchestrator::handleInStorStorageMoveToSlotCmd() {
    if (!onStateEnter("Orchestrator: Reserving storage slot...\n", true)) {
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
            transitionTo(OrchestratorState::InStor_RobotOverStorage_Cmd);
            break;
        case CmdStatus::FAILED:
            transitionToFault("Gripper failed to close");
            break;
        default:
            break;
    }
}

void Orchestrator::handleInStorRobotOverStorageCmd() {

}

void Orchestrator::handleInStorRobotOverStorageWait() {

}

void Orchestrator::handleInStorStorageMoveToPosCmd() {

}

void Orchestrator::handleInStorStorageMoveToPosWait() {

}

void Orchestrator::handleInStorRobotDownToSlotCmd() {

}

void Orchestrator::handleInStorRobotDownToSlotWait() {

}

void Orchestrator::handleInStorGripperOpenCmd() {

}

void Orchestrator::handleInStorGripperOpenWait() {

}

void Orchestrator::handleInStorRobotUpFromSlotCmd() {

}

void Orchestrator::handleInStorRobotUpFromSlotWait() {

}

void Orchestrator::handleInStorComplete() {

}

void Orchestrator::handleStopping() {

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

