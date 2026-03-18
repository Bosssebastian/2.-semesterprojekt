#include "Gripper.h"
#include "config/ParameterConfig.h"

Gripper::Gripper(StepperAxis& axis)
    : mAxis(axis) {}

void Gripper::begin() {
    mMoveStep = MoveStep::Idle;
    mIsBusy = false;
    mLastResult = GripperResult::None;
    mAxis.setEnabled(false);
}

bool Gripper::open(bool stopOnStall) {
    if (mIsBusy) {
        return false;
    }

    mLastResult = GripperResult::None;
    mIsBusy = true;
    return startMoveStep(MoveStep::Open, -ParameterConfig::GRIPPER_OPEN_STEPS, stopOnStall);
}

bool Gripper::close(bool stopOnStall) {
    if (mIsBusy) {
        return false;
    }

    mLastResult = GripperResult::None;
    mIsBusy = true;
    return startMoveStep(MoveStep::Close, ParameterConfig::GRIPPER_CLOSE_STEPS, stopOnStall);
}

void Gripper::stop() {
    if (!mIsBusy) {
        return;
    }

    mAxis.stop();
    endMove(GripperResult::Stopped, false);
}

void Gripper::update() {
    if (!mIsBusy || mAxis.isBusy()) {
        return;
    }

    const AxisMoveResult axisResult = mAxis.getLastMoveResult();

    switch (mMoveStep) {
        case MoveStep::Close:
            if (axisResult == AxisMoveResult::Stalled) {
                endMove(GripperResult::Stalled, true);
            } else if (axisResult == AxisMoveResult::Done) {
                endMove(GripperResult::Done, true);
            } else {
                endMove(GripperResult::Stopped, false);
            }
            return;

        case MoveStep::Open:
            if (axisResult == AxisMoveResult::Stalled) {
                endMove(GripperResult::Stalled, false);
            } else if (axisResult == AxisMoveResult::Done) {
                endMove(GripperResult::Done, false);
            } else {
                endMove(GripperResult::Stopped, false);
            }
            return;

        case MoveStep::Idle:
            return;
    }
}

bool Gripper::isBusy() const {
    return mIsBusy;
}

GripperResult Gripper::getLastResult() const {
    return mLastResult;
}

bool Gripper::startMoveStep(MoveStep moveStep, int32_t steps, bool stopOnStall) {
    mMoveStep = moveStep;

    if (steps == 0 || !mAxis.move(steps, stopOnStall)) {
        mMoveStep = MoveStep::Idle;
        mIsBusy = false;
        return false;
    }

    return true;
}

void Gripper::endMove(GripperResult result, bool keepMotorEnabled) {
    mMoveStep = MoveStep::Idle;
    mIsBusy = false;
    mLastResult = result;

    if (!keepMotorEnabled) {
        mAxis.setEnabled(false);
    }
}
