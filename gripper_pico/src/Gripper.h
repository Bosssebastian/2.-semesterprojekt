#pragma once

#include <cstdint>

#include "StepperAxis.h"

enum class GripperResult : uint8_t {None, Done, Stalled, Stopped, Error};

class Gripper {
public:
    explicit Gripper(StepperAxis& axis);

    void begin();
    bool open(bool stopOnStall = false);
    bool close(bool stopOnStall = false);
    void stop();
    void update();

    bool isBusy() const;
    GripperResult getLastResult() const;

private:
    enum class MoveStep : uint8_t {Idle, Close, Open};

    StepperAxis& mAxis;
    MoveStep mMoveStep = MoveStep::Idle;
    bool mIsBusy = false;
    GripperResult mLastResult = GripperResult::None;

    bool startMoveStep(MoveStep moveStep, int32_t steps, bool stopOnStall);
    void endMove(GripperResult result, bool keepMotorEnabled);
};
