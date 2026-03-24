#include "Orchestrator.h"
#include "io/IoRunner.h"
#include "vision/VisionRunner.h"
#include <iostream>
#include <limits>


Orchestrator::Orchestrator(IoRunner& io, VisionRunner& vision)
    : mIo(io), mVision(vision) {
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
}

void Orchestrator::transitionToFault(const std::string& reason) {
    mFaultReason = reason;
    transitionTo(OrchestratorState::Faulted);
}

void Orchestrator::handleStarting() {
    // Startup logic here
    printf("Orchestrator: Starting up...\n");
    waitForEnter();
    transitionTo(OrchestratorState::Idle);
}

void Orchestrator::handleIdle() {
    printf("Orchestrator: Idle. Waiting for cube on input...\n");
    // Check for cube on input, e.g. via vision
    // If cube detected, transition to next state
    waitForEnter();
    transitionTo(OrchestratorState::InStor_StorageMoveToSlot_Cmd);
}

void Orchestrator::handleInStorStorageMoveToSlotCmd() {
    printf("Orchestrator: Reserving storage slot...\n");
    // Logic to reserve a storage slot for the incoming cube
    // If successful, transition to next state
    waitForEnter();
    transitionTo(OrchestratorState::InStor_RobotOverInput_Cmd);
}

void Orchestrator::handleInStorRobotOverInputCmd() {
    printf("Orchestrator: Commanding move over input...\n");
    // Command robot to move over input area
    // If command successful, transition to next state
    waitForEnter();
    transitionTo(OrchestratorState::InStor_RobotOverInput_Wait);

}

void Orchestrator::handleInStorRobotOverInputWait() {
    printf("Orchestrator: Waiting for move over input to complete...\n");
    // Check if robot has completed move over input
    // If completed, transition to next state
    waitForEnter();
    transitionTo(OrchestratorState::InStor_RobotToCube_Cmd);

}

void Orchestrator::handleInStorRobotToCubeCmd() {

}

void Orchestrator::handleInStorRobotToCubeWait() {

}

void Orchestrator::handleInStorGripperCloseCmd() {

}

void Orchestrator::handleInStorGripperCloseWait() {

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


void Orchestrator::waitForEnter() {
    printf("Press Enter to continue...");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
