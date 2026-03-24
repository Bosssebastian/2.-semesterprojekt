#pragma once
#include <string>

class IoRunner;
class VisionRunner;

enum class OrchestratorState {
    Starting,
    Idle,
    InStor_StorageMoveToSlot_Cmd,

    InStor_RobotOverInput_Cmd,
    InStor_RobotOverInput_Wait,

    InStor_RobotToCube_Cmd,
    InStor_RobotToCube_Wait,

    InStor_GripperClose_Cmd,
    InStor_GripperClose_Wait,

    InStor_RobotOverStorage_Cmd,
    InStor_RobotOverStorage_Wait,

    InStor_StorageMoveToPos_Cmd,
    InStor_StorageMoveToPos_Wait,

    InStor_RobotDownToSlot_Cmd,
    InStor_RobotDownToSlot_Wait,

    InStor_GripperOpen_Cmd,
    InStor_GripperOpen_Wait,

    InStor_RobotUpFromSlot_Cmd,
    InStor_RobotUpFromSlot_Wait,

    InStor_Complete,
    Stopping,
    Stopped,
    Faulted
};

class Orchestrator {
public:
    Orchestrator(IoRunner& io, VisionRunner& vision);

    void run();

    OrchestratorState getState() const;
    const std::string& getFaultReason() const;

private:
    void start();
    void update();

    void transitionTo(OrchestratorState newState);
    void transitionToFault(const std::string& reason);

    void handleStarting();
    void handleIdle();
    void handleInStorStorageMoveToSlotCmd();

    void handleInStorRobotOverInputCmd();
    void handleInStorRobotOverInputWait();

    void handleInStorRobotToCubeCmd();
    void handleInStorRobotToCubeWait();

    void handleInStorGripperCloseCmd();
    void handleInStorGripperCloseWait();

    void handleInStorRobotOverStorageCmd();
    void handleInStorRobotOverStorageWait();

    void handleInStorStorageMoveToPosCmd();
    void handleInStorStorageMoveToPosWait();

    void handleInStorRobotDownToSlotCmd();
    void handleInStorRobotDownToSlotWait();

    void handleInStorGripperOpenCmd();
    void handleInStorGripperOpenWait();

    void handleInStorRobotUpFromSlotCmd();
    void handleInStorRobotUpFromSlotWait();

    void handleInStorComplete();
    void handleStopping();

private:
    IoRunner& mIo;
    VisionRunner& mVision;

    OrchestratorState mState{OrchestratorState::Starting};
    std::string mFaultReason;
    bool mStopRequested{false};

    void waitForEnter();
};
