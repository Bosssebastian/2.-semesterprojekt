#include "Orchestrator.h"
#include "io/IoRunner.h"
#include "logging/Logger.h"
#include "web/WebServerRunner.h"
#include "vision/VisionRunner.h"
#include "Types.h"
#include <chrono>
#include <cstdio>
#include <thread>
#include "movement.h"

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
}

Orchestrator::Orchestrator(IoRunner& io, VisionRunner& vision, WebServerRunner& web)
    : mIo(io), mVision(vision), mWeb(web), mGripper(io.gripper()), mStorage(io.storage()), mDatabase("storage.db"), mStorageManager(mDatabase), mMove() {
    mWeb.setState(mState);
    publishStorageSlotStates();
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
    
    mDatabase.createTables();
    
    transitionTo(OrchestratorState::Stopped);
}


void Orchestrator::update() {
    publishStorageSlotStates();

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
        case OrchestratorState::InStor_GetStorageSlot: handleInStorGetStorageSlot(); break;
        case OrchestratorState::InStor_StorageMoveToSlot: handleInStorStorageMoveToSlot(); break;
        case OrchestratorState::InStor_RobotOverInput: handleInStorRobotOverInput(); break;
        case OrchestratorState::InStor_RobotToCube: handleInStorRobotToCube(); break;
        case OrchestratorState::InStor_GripperClose: handleInStorGripperClose(); break;
        case OrchestratorState::InStor_RobotOverStorage: handleInStorRobotOverStorage(); break;
        case OrchestratorState::InStor_RobotOverStorage2: handleInStorRobotOverStorage2(); break;
        case OrchestratorState::InStor_StorageWaitingOnMove: handleStorageWaitingOnMove(); break;
        case OrchestratorState::InStor_RobotDownToSlot: handleInStorRobotDownToSlot(); break;
        case OrchestratorState::InStor_GripperOpen: handleInStorGripperOpen(); break;
        case OrchestratorState::InStor_RobotUpFromSlot: handleInStorRobotUpFromSlot(); break;
        case OrchestratorState::InStor_Complete: handleInStorComplete(); break;

        case OrchestratorState::OutStor_StorageMoveToSlot: handleOutStorStorageMoveToSlot(); break;
        case OrchestratorState::OutStor_RobotOverStorage: handleOutStorRobotOverStorage(); break;
        case OrchestratorState::OutStor_StorageWaitingOnMove: handleOutStorStorageWaitingOnMove(); break;
        case OrchestratorState::OutStor_RobotDownToSlot: handleOutStorRobotDownToSlot(); break;
        case OrchestratorState::OutStor_GripperClose: handleOutStorGripperClose(); break;
        case OrchestratorState::OutStor_RobotUpFromSlot: handleOutStorRobotUpFromSlot(); break;
        case OrchestratorState::OutStor_RobotOverOutput: handleOutStorRobotOverOutput(); break;
        case OrchestratorState::OutStor_GripperOpen: handleOutStorGripperOpen(); break;
        case OrchestratorState::OutStor_Complete: handleOutStorComplete(); break;
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

void Orchestrator::publishStorageSlotStates() {
    mWeb.setStorageSlotStates(mStorageManager.getSlotStates());
}

void Orchestrator::handleWebCommand(const WebCommand& command) {
    switch (command.type) {
        case WebCommandType::Start:
            if (mState == OrchestratorState::Stopped) {
                LOG_INFO("Start command received");
                transitionTo(OrchestratorState::Starting);
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
        case WebCommandType::StorageSlotGoto:
            if (mState == OrchestratorState::Idle && command.slotIndex >= 0 && command.slotIndex < 8) {
                if (!mStorageManager.isOccupied(command.slotIndex)) {
                    LOG_WARN("Storage-to-output command ignored because slot " + std::to_string(command.slotIndex + 1) + " is free");
                    break;
                }
                mVision.stopScan();
                mActiveStorageSlot = command.slotIndex;
                LOG_INFO("Storage-to-output command received: slot " + std::to_string(mActiveStorageSlot + 1));
                transitionTo(OrchestratorState::OutStor_StorageMoveToSlot);
            }
            break;
    }
}

bool Orchestrator::skipRequested() {
    if (mPendingSkipRequest) {
        mPendingSkipRequest = false;
        return true;
    }
    return false;
}

bool Orchestrator::gripperReadyForIdle() {
    switch (mGripper.getStatus(CmdType::STATUS)) {
        case CmdStatus::DONE:
            if (!mGripper.isDeviceBusy()) {
                LOG_INFO("Gripper reports idle; cycle can return to idle.");
                return true;
            }
            mGripper.sendCommand(CmdType::STATUS);
            return false;
        case CmdStatus::FAILED:
            transitionToFault("Gripper status query failed");
            return false;
        case CmdStatus::TIMED_OUT:
            transitionToFault("Gripper status query timed out");
            return false;
        case CmdStatus::WAITING_FOR_ACK:
        case CmdStatus::WAITING_FOR_RESULT:
            return false;
        case CmdStatus::IDLE:
            mGripper.sendCommand(CmdType::STATUS);
            return false;
    }

    return false;
}

void Orchestrator::handleStarting() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Starting up...");
        mGripper.resetCommandStates();
        mStorage.resetCommandStates();
        mGripper.sendCommand(CmdType::RESET);
        mMove.home(); // Move to home pose
        mMove.setTransform(); // Set up transformation matrices
    });

    switch (mGripper.getStatus(CmdType::RESET)) {
        case CmdStatus::DONE:
            LOG_INFO("Gripper reset completed successfully.");
            transitionTo(OrchestratorState::Idle);
            break;
        case CmdStatus::FAILED:
            transitionToFault("Gripper failed to reset");
            break;
        case CmdStatus::TIMED_OUT:
            transitionToFault("Gripper reset timed out");
            break;
        default:
            break;
    }
    //transitionTo(OrchestratorState::Idle);
}

void Orchestrator::handleIdle() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Idle. Waiting for input...");
        double speed = 0.2;
        double acc = 0.1;
        mMove.home(speed,acc);
        mVision.scanForObject();
    });

    if (mVision.objectReady()){
        LOG_INFO("Orchestrator: Object found");
        mVision.getPos(mInputFromVision, mRot);
        mVision.getInfo(mObject, mSize, mColor);
        mActiveObjectId = mDatabase.insertVisionObject(mObject, mSize, mColor);// data input form vision (object type, size, color) 
        transitionTo(OrchestratorState::InStor_GetStorageSlot);
    }

    if (skipRequested()) {
        mVision.stopScan();
        transitionTo(OrchestratorState::InStor_GetStorageSlot);
        return;
    }
}

void Orchestrator::handleInStorGetStorageSlot() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Getting storage slot...");
    });

    if (!mStorageManager.hasFreeSlot()) {
        transitionToFault("No free storage slots available");
        return;
    }

    mActiveStorageSlot = mStorageManager.getFreeSlot();
    //mStorageManager.occupySlot(mActiveStorageSlot, mActiveObjectId);
    mDatabase.updateStorageSlot(mActiveStorageSlot, mActiveObjectId, true); //(int slotId, int objectId, bool occupied) 0 empty 1 occupied
    mDatabase.insertHistory(mActiveObjectId, mActiveStorageSlot, mDatabase.time()); // (int objectId, int slot, double timestamp)
    LOG_INFO("Storage: Using storage slot " + std::to_string(mActiveStorageSlot + 1)); // 0-7 incorrect logic, 1-8
    transitionTo(OrchestratorState::InStor_StorageMoveToSlot);
}

void Orchestrator::handleInStorStorageMoveToSlot() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage move to slot "+ std::to_string(mActiveStorageSlot + 1) + " state.");
        mStorage.sendCommand(CmdType::GOTO, std::to_string(mActiveStorageSlot + 1));
    });
    transitionTo(OrchestratorState::InStor_RobotOverInput);
}

void Orchestrator::handleInStorRobotOverInput() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Commanding move over input...");
        MatrixOperations matop;
        double speed = 0.3;
        double acc = 0.2;
        double customZ = 0.18;
        double rotZ = matop.degToRad(22.5) - mRot;
        mMove.stop();
        mMove.move({mInputFromVision[0],mInputFromVision[1],{0.24}},speed,acc,customZ,rotZ); // Move to coordinates provided by vision
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotToCube);
        return;
    }

    if (mMove.isDone()){ // Check if async movement is done
        transitionTo(OrchestratorState::InStor_RobotToCube);
    }
}

void Orchestrator::handleInStorRobotToCube() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Commanding move to cube...");
        mMove.moveDown("base"); // Moves down
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_GripperClose);
        return;
    }

    if (mMove.isDone()){ // Check if async movement is done
        transitionTo(OrchestratorState::InStor_GripperClose);
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
    onEnter([this] {
        LOG_INFO("Orchestrator: Move robot over storage.");
        mStorageManager.occupySlot(mActiveStorageSlot, mActiveObjectId);
        mMove.moveUp("base"); // part 2 of movement in handleInStorRobotOverStorage2()
        //mMove.moveUp("home");
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotOverStorage2);
        return;
    }

    if (mMove.isDone()){ // Check if async movement is done
        transitionTo(OrchestratorState::InStor_RobotOverStorage2);
    }
}

void Orchestrator::handleInStorRobotOverStorage2() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage move to position placeholder state.");
        mMove.moveJStock("storage"); // part 1 of movement in handleInStorRobotOverStorage()
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_StorageWaitingOnMove);
        return;
    }
    if (mMove.isDone()){ // Check if async movement is done
        transitionTo(OrchestratorState::InStor_StorageWaitingOnMove);
    }
}

void Orchestrator::handleStorageWaitingOnMove() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage move to position placeholder state.");
    });

    if (skipRequested()) {
        mStorage.resetCommandStates();
        transitionTo(OrchestratorState::InStor_RobotDownToSlot);
        return;
    }
    switch (mStorage.getStatus(CmdType::GOTO)) {
        case CmdStatus::DONE:
            LOG_INFO("Storage System move completed successfully.");
            transitionTo(OrchestratorState::InStor_RobotDownToSlot);
            break;
        case CmdStatus::FAILED:
            transitionToFault("Storage System failed to move");
            break;
        case CmdStatus::TIMED_OUT:
            transitionToFault("Storage System move timed out");
            break;
        default:
            break;
    }
}

void Orchestrator::handleInStorRobotDownToSlot() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Robot down to slot placeholder state.");
        mMove.moveDown("storage"); // Move to put object down in storage
    });

    if (skipRequested()) {   
        transitionTo(OrchestratorState::InStor_GripperOpen);
        return;
    }

    if (mMove.isDone()){ // Check if async movement is done
        transitionTo(OrchestratorState::InStor_GripperOpen);
    }
}

void Orchestrator::handleInStorGripperOpen() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Commanding gripper open...");
        mGripper.sendCommand(CmdType::OPEN);
        mStorageManager.occupySlot(mActiveStorageSlot, mActiveObjectId);
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_RobotUpFromSlot);
        return;
    }

    switch (mGripper.getStatus(CmdType::OPEN)) {
        case CmdStatus::DONE:
            LOG_INFO("Gripper open completed successfully.");
            transitionTo(OrchestratorState::InStor_RobotUpFromSlot);
            break;
        case CmdStatus::FAILED:
            transitionToFault("Gripper failed to open");
            break;
        case CmdStatus::TIMED_OUT:
            transitionToFault("Gripper open timed out");
            break;
        default:
            break;
    }
}

void Orchestrator::handleInStorRobotUpFromSlot() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Robot up from slot placeholder state.");
        //mMove.moveUp("storage");
        mMove.moveJStock("storage");
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::InStor_Complete);
        return;
    }

    if (mMove.isDone()){ // Check if async movement is done
        transitionTo(OrchestratorState::InStor_Complete);
    }
}

void Orchestrator::handleInStorComplete() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Input-to-storage cycle complete.");
        mGripper.sendCommand(CmdType::STATUS);
    });
    if (gripperReadyForIdle()) {
        transitionTo(OrchestratorState::Idle);
    }
}

void Orchestrator::handleOutStorStorageMoveToSlot() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage-to-output move storage to slot " + std::to_string(mActiveStorageSlot + 1));
        mStorage.sendCommand(CmdType::GOTO, std::to_string(mActiveStorageSlot + 1));
    });
    transitionTo(OrchestratorState::OutStor_RobotOverStorage);
}

void Orchestrator::handleOutStorRobotOverStorage() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage-to-output move robot over storage.");
        mMove.moveJStock("storage");
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::OutStor_StorageWaitingOnMove);
        return;
    }

    if (mMove.isDone()){ // Check if async movement is done
        transitionTo(OrchestratorState::OutStor_StorageWaitingOnMove);
    }

}

void Orchestrator::handleOutStorStorageWaitingOnMove() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Waiting on storage to reach selected slot.");
    });

    if (skipRequested()) {
        mStorage.resetCommandStates();
        transitionTo(OrchestratorState::OutStor_RobotDownToSlot);
        return;
    }

    switch (mStorage.getStatus(CmdType::GOTO)) {
        case CmdStatus::DONE:
            LOG_INFO("Storage System move completed successfully.");
            transitionTo(OrchestratorState::OutStor_RobotDownToSlot);
            break;
        case CmdStatus::FAILED:
            transitionToFault("Storage System failed to move");
            break;
        case CmdStatus::TIMED_OUT:
            transitionToFault("Storage System move timed out");
            break;
        default:
            break;
    }
}

void Orchestrator::handleOutStorRobotDownToSlot() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage-to-output robot down to selected slot.");
        mMove.moveDown("storage");
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::OutStor_GripperClose);
        return;
    }

    if (mMove.isDone()) {
        transitionTo(OrchestratorState::OutStor_GripperClose);
    }
}

void Orchestrator::handleOutStorGripperClose() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage-to-output closing gripper.");
        mGripper.sendCommand(CmdType::CLOSE);
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::OutStor_RobotUpFromSlot);
        return;
    }

    switch (mGripper.getStatus(CmdType::CLOSE)) {
        case CmdStatus::DONE:
            LOG_INFO("Gripper close completed successfully.");
            transitionTo(OrchestratorState::OutStor_RobotUpFromSlot);
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

void Orchestrator::handleOutStorRobotUpFromSlot() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage-to-output robot up from selected slot.");
        mMove.moveJStock("storage");
        mStorageManager.freeSlot(mActiveStorageSlot);
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::OutStor_RobotOverOutput);
        return;
    }

    if (mMove.isDone()) {
        transitionTo(OrchestratorState::OutStor_RobotOverOutput);
    }
}

void Orchestrator::handleOutStorRobotOverOutput() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage-to-output moving robot to output placeholder.");
        double acc = 0.2;
        double speed = 0.5;
        bool test = true;
        mMove.toOutput(speed,acc,test);
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::OutStor_GripperOpen);
        return;
    }

    if (mMove.isDone()) {
        transitionTo(OrchestratorState::OutStor_GripperOpen);
    }
}

void Orchestrator::handleOutStorGripperOpen() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage-to-output opening gripper.");
        mGripper.sendCommand(CmdType::OPEN);
    });

    if (skipRequested()) {
        transitionTo(OrchestratorState::OutStor_Complete);
        return;
    }

    switch (mGripper.getStatus(CmdType::OPEN)) {
        case CmdStatus::DONE:
            LOG_INFO("Gripper open completed successfully.");
            transitionTo(OrchestratorState::OutStor_Complete);
            break;
        case CmdStatus::FAILED:
            transitionToFault("Gripper failed to open");
            break;
        case CmdStatus::TIMED_OUT:
            transitionToFault("Gripper open timed out");
            break;
        default:
            break;
    }
}

void Orchestrator::handleOutStorComplete() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Storage-to-output cycle complete.");
        //mStorageManager.freeSlot(mActiveStorageSlot);
        mGripper.sendCommand(CmdType::STATUS);
        mMove.moveUp("output");
    });
    if (gripperReadyForIdle() && mMove.isDone()) {
        transitionTo(OrchestratorState::Idle);
    }
}

void Orchestrator::handleResetting() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Resetting system...");
        mGripper.resetCommandStates();
        mStorage.resetCommandStates();
    });
    mMove.home();
    stopMotion();
    mFaultReason.clear();
    mPendingSkipRequest = false;
    transitionTo(OrchestratorState::Stopped);
}

void Orchestrator::handleStopping() {
    onEnter([this] {
        LOG_INFO("Orchestrator: Stopping system...");
        mVision.stopScan();
    });
    stopMotion();
    transitionTo(OrchestratorState::Stopped);
}

void Orchestrator::stopMotion() {
    mGripper.sendCommand(CmdType::STOP);
    mStorage.sendCommand(CmdType::STOP);
    mMove.stop(); // Stops script on robot
    }
