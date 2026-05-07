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
    bool skipRequested();

    void transitionTo(OrchestratorState newState);
    void transitionToFault(const std::string& reason);

    template <typename Func>
    void onEnter(Func&& func) {
        if (!mStateJustEntered) {
            return;
        }

        func();
        mStateJustEntered = false;
    }

    void handleStarting();
    void handleIdle();
    void handleInStorStorageMoveToSlot();
    void handleInStorRobotOverInput();
    void handleInStorRobotToCube();
    void handleInStorGripperClose();
    void handleInStorRobotOverStorage();
    void handleInStorStorageMoveToPos();
    void handleInStorRobotDownToSlot();
    void handleInStorGripperOpen();
    void handleInStorRobotUpFromSlot();

    void handleInStorComplete();
    void handleResetting();
    void handleStopping();
    void stopMotion();

private:
    IoRunner& mIo;
    VisionRunner& mVision;
    WebServerRunner& mWeb;
    Interface& mGripper;
    Interface& mStorage;

    OrchestratorState mState{OrchestratorState::Stopped};
    std::string mFaultReason;
    bool mStopRequested{false};
    bool mPendingSkipRequest{false};
    bool mStateJustEntered{true};
};
