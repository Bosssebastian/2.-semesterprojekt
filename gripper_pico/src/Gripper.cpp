#include "Gripper.h"
#include "config/ParameterConfig.h"
#include "config/PinConfig.h"

Gripper::Gripper()
    : mDriver(PinConfig::TMC_UART_PORT, PinConfig::TMC_UART_BAUD, PinConfig::TMC_ADDRESS),
      mAxis(mDriver) {}

void Gripper::setup() {
    mDriver.setup();
    mDriver.setCurrent(ParameterConfig::DRIVER_HOLD_CURRENT, ParameterConfig::DRIVER_RUN_CURRENT, ParameterConfig::DRIVER_HOLD_DELAY);
    mDriver.setMicrosteps(ParameterConfig::DRIVER_MICROSTEPS);
    mDriver.enableSpreadCycle(ParameterConfig::DRIVER_SPREAD_CYCLE_ENABLED);
    mDriver.setPwmThreshold(ParameterConfig::DRIVER_PWM_THRESHOLD);
    mDriver.configureStallGuard(ParameterConfig::DRIVER_COOL_THRESHOLD, ParameterConfig::DRIVER_STALL_THRESHOLD);

    mAxis.setup();
    mMoveStep = MoveStep::Idle;
    mActiveCommand = CmdType::NONE;
    mIsBusy = false;
    mLastResult = GripperMoveResult::None;
    mHasMoveEvent = false;
    mMoveEvent = {};
    mAxis.setEnabled(false);
}

bool Gripper::open(bool stopOnStall) {
    if (mIsBusy) {
        return false;
    }

    mLastResult = GripperMoveResult::None;
    mActiveCommand = CmdType::OPEN;
    mIsBusy = true;
    return startMoveStep(MoveStep::Open, -ParameterConfig::GRIPPER_OPEN_STEPS, stopOnStall);
}

bool Gripper::close(bool stopOnStall) {
    if (mIsBusy) {
        return false;
    }

    mLastResult = GripperMoveResult::None;
    mActiveCommand = CmdType::CLOSE;
    mIsBusy = true;
    return startMoveStep(MoveStep::Close, ParameterConfig::GRIPPER_CLOSE_STEPS, stopOnStall);
}

void Gripper::stop() {
    if (!mIsBusy) {
        mAxis.setEnabled(false);
        return;
    }

    mAxis.stop();
    endMove(GripperMoveResult::Stopped, false);
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
                endMove(GripperMoveResult::Stalled, true);
            } else if (axisResult == AxisMoveResult::Done) {
                endMove(GripperMoveResult::Done, true);
            } else {
                endMove(GripperMoveResult::Stopped, false);
            }
            return;

        case MoveStep::Open:
            if (axisResult == AxisMoveResult::Stalled) {
                endMove(GripperMoveResult::Stalled, false);
            } else if (axisResult == AxisMoveResult::Done) {
                endMove(GripperMoveResult::Done, false);
            } else {
                endMove(GripperMoveResult::Stopped, false);
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

GripperMoveResult Gripper::getLastResult() const {
    return mLastResult;
}

bool Gripper::hasMoveEvent() const {
    return mHasMoveEvent;
}

GripperMoveEvent Gripper::getMoveEvent() {
    if (!mHasMoveEvent) {
        return {};
    }

    mHasMoveEvent = false;
    const GripperMoveEvent event = mMoveEvent;
    mMoveEvent = {};
    return event;
}

bool Gripper::startMoveStep(MoveStep moveStep, int32_t steps, bool stopOnStall) {
    mMoveStep = moveStep;

    if (steps == 0) {
        mMoveStep = MoveStep::Idle;
        mActiveCommand = CmdType::NONE;
        mIsBusy = false;
        return false;
    }

    if (!mAxis.move(steps, stopOnStall)) {
        mMoveStep = MoveStep::Idle;
        mActiveCommand = CmdType::NONE;
        mIsBusy = false;
        return false;
    }

    return true;
}

void Gripper::endMove(GripperMoveResult result, bool keepMotorEnabled) {
    const CmdType completedCommand = mActiveCommand;

    mMoveStep = MoveStep::Idle;
    mActiveCommand = CmdType::NONE;
    mIsBusy = false;
    mLastResult = result;
    if (completedCommand == CmdType::OPEN || completedCommand == CmdType::CLOSE) {
        mMoveEvent.cmd = completedCommand;
        mMoveEvent.result = result;
        mHasMoveEvent = true;
    }

    if (!keepMotorEnabled) {
        mAxis.setEnabled(false);
    }
}
