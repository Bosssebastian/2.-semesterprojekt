#pragma once

#include <cstdint>

#include "stepper/StepperAxis.h"
#include "stepper/TMC2209Driver.h"

enum class GripperResult : uint8_t {None, Done, Stalled, Stopped, Error};

class Gripper {
public:
    Gripper();
    bool setup();
    void update();

    bool open(bool stopOnStall = false);
    bool close(bool stopOnStall = false);
    void stop();
    TMC2209Driver& driver();
    const TMC2209Driver& driver() const;
    StepperAxis& axis();
    const StepperAxis& axis() const;

    bool isBusy() const;
    GripperResult getLastResult() const;

private:
    enum class MoveStep : uint8_t {Idle, Close, Open};

    TMC2209Driver mDriver;
    StepperAxis mAxis;
    MoveStep mMoveStep = MoveStep::Idle;
    bool mIsBusy = false;
    GripperResult mLastResult = GripperResult::None;

    bool startMoveStep(MoveStep moveStep, int32_t steps, bool stopOnStall);
    void endMove(GripperResult result, bool keepMotorEnabled);
};
