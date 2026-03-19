#include "Gripper.h"
#include "config/ParameterConfig.h"
#include "config/PinConfig.h"

Gripper::Gripper()
    : mDriver(PinConfig::TMC_UART_PORT, PinConfig::TMC_UART_BAUD, PinConfig::TMC_ADDRESS),
      mAxis(mDriver) {}

bool Gripper::setup() {
    mDriver.setup();
    const bool driverConfigured =
        mDriver.setCurrent(ParameterConfig::DRIVER_HOLD_CURRENT, ParameterConfig::DRIVER_RUN_CURRENT, ParameterConfig::DRIVER_HOLD_DELAY) &&
        mDriver.setMicrosteps(ParameterConfig::DRIVER_MICROSTEPS) &&
        mDriver.enableSpreadCycle(ParameterConfig::DRIVER_SPREAD_CYCLE_ENABLED) &&
        mDriver.setPwmThreshold(ParameterConfig::DRIVER_PWM_THRESHOLD) &&
        mDriver.configureStallGuard(ParameterConfig::DRIVER_COOL_THRESHOLD, ParameterConfig::DRIVER_STALL_THRESHOLD);

    mAxis.setup();
    mMoveStep = MoveStep::Idle;
    mIsBusy = false;
    mLastResult = GripperResult::None;
    mAxis.setEnabled(false);
    return driverConfigured;
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
    mAxis.update();

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

TMC2209Driver& Gripper::driver() {
    return mDriver;
}

const TMC2209Driver& Gripper::driver() const {
    return mDriver;
}

StepperAxis& Gripper::axis() {
    return mAxis;
}

const StepperAxis& Gripper::axis() const {
    return mAxis;
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
