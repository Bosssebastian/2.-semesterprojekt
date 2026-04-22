#pragma once

#include <cstdint>

#include "../../shared/Types.h"
#include "stepper/StepperAxis.h"
#include "stepper/TMC2209Driver.h"

enum class GripperMoveResult : uint8_t {None, Done, Stalled, Stopped, Error};

inline const char* toString(GripperMoveResult result) {
    switch (result) {
        case GripperMoveResult::None:
            return "None";
        case GripperMoveResult::Done:
            return "Done";
        case GripperMoveResult::Stalled:
            return "Stalled";
        case GripperMoveResult::Stopped:
            return "Stopped";
        case GripperMoveResult::Error:
            return "Error";
    }

    return "Unknown";
}

struct GripperMoveEvent {
    CmdType cmd = CmdType::NONE;
    GripperMoveResult result = GripperMoveResult::None;
};

class Gripper {
public:
    Gripper();
    void setup();
    void update();

    bool open(bool stopOnStall = false);
    bool close(bool stopOnStall = false);
    void stop();
    TMC2209Driver& driver();
    const TMC2209Driver& driver() const;
    StepperAxis& axis();
    const StepperAxis& axis() const;

    bool isBusy() const;
    GripperMoveResult getLastResult() const;
    bool hasMoveEvent() const;
    GripperMoveEvent getMoveEvent();

private:
    enum class MoveStep : uint8_t {Idle, Close, Open};

    TMC2209Driver mDriver;
    StepperAxis mAxis;
    MoveStep mMoveStep = MoveStep::Idle;
    CmdType mActiveCommand = CmdType::NONE;
    bool mIsBusy = false;
    GripperMoveResult mLastResult = GripperMoveResult::None;
    bool mHasMoveEvent = false;
    GripperMoveEvent mMoveEvent = {};

    bool startMoveStep(MoveStep moveStep, int32_t steps, bool stopOnStall);
    void endMove(GripperMoveResult result, bool keepMotorEnabled);
};
