#pragma once
#include "OrchestratorState.h"
#include "io/Interface.h"
#include <string>

class IoRunner;
class VisionRunner;
class WebServerRunner;
struct WebCommand;

class Orchestrator {
public:
    Orchestrator(IoRunner& io, VisionRunner& vision, WebServerRunner& web);

    void run();

    OrchestratorState getState() const;
    const std::string& getFaultReason() const;

private:
    void start();
    void update();
    void handleWebCommand(const WebCommand& command);

    void transitionTo(OrchestratorState newState);
    void transitionToFault(const std::string& reason);
    bool onStateEnter(const char* message, bool waitForEnter = false);

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
    WebServerRunner& mWeb;
    Interface& mGripper;
    Interface& mStorage;

    OrchestratorState mState{OrchestratorState::Stopped};
    std::string mFaultReason;
    std::string mRequestedObjectId;
    bool mStopRequested{false};
    bool mStateJustEntered{true};
};
