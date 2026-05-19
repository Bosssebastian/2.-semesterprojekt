#include "Gripper.h"
#include "config/ParameterConfig.h"
#include "config/PinConfig.h"

Gripper::Gripper(CurrentSensor& currentSensor)
    : mDriver(PinConfig::TMC_UART_PORT, PinConfig::TMC_UART_BAUD, PinConfig::TMC_ADDRESS),
      mAxis(mDriver, currentSensor) {}

void Gripper::setup() {
    mDriver.setup();
    mDriver.setCurrent(ParameterConfig::DRIVER_HOLD_CURRENT, ParameterConfig::DRIVER_RUN_CURRENT, ParameterConfig::DRIVER_HOLD_DELAY);
    mDriver.setMicrosteps(ParameterConfig::DRIVER_MICROSTEPS);
    mDriver.enableSpreadCycle(ParameterConfig::DRIVER_SPREAD_CYCLE_ENABLED);
    mDriver.setPwmThreshold(ParameterConfig::DRIVER_PWM_THRESHOLD);
    mDriver.configureStallGuard(ParameterConfig::DRIVER_COOL_THRESHOLD, ParameterConfig::DRIVER_STALL_GUARD_REGISTER_VALUE);

    mAxis.setup();
    mMoveStep = MoveStep::Idle;
    mActiveCommand = CmdType::NONE;
    mSilentReset = false;
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
    mSilentReset = false;
    mActiveCommand = CmdType::OPEN;
    mIsBusy = true;
    return startMoveStep(MoveStep::Open, -ParameterConfig::GRIPPER_OPEN_STEPS, stopOnStall);
}

bool Gripper::close(bool stopOnStall) {
    if (mIsBusy) {
        return false;
    }

    mLastResult = GripperMoveResult::None;
    mSilentReset = false;
    mActiveCommand = CmdType::CLOSE;
    mIsBusy = true;
    return startMoveStep(MoveStep::Close, ParameterConfig::GRIPPER_CLOSE_STEPS, stopOnStall);
}

bool Gripper::reset() {
    if (mIsBusy) {
        return false;
    }

    mLastResult = GripperMoveResult::None;
    mSilentReset = false;
    mActiveCommand = CmdType::RESET;
    mIsBusy = true;
    return startResetSequence(false);
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

    if (mIsBusy && mMoveStep == MoveStep::OpenResetPause) {
        if (absolute_time_diff_us(get_absolute_time(), mPauseUntil) > 0) {
            return;
        }

        if (!startResetSequence(true)) {
            endMove(GripperMoveResult::Error, false);
        }
        return;
    }

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
                publishMoveEvent(CmdType::OPEN, GripperMoveResult::Stalled);
                mSilentReset = true;
                if (!startMoveStep(MoveStep::ResetForward, ParameterConfig::GRIPPER_RESET_FORWARD_STEPS, false)) {
                    endMove(GripperMoveResult::Error, false);
                }
            } else if (axisResult == AxisMoveResult::Done) {
                publishMoveEvent(CmdType::OPEN, GripperMoveResult::Done);
                if (!startResetSequence(true)) {
                    endMove(GripperMoveResult::Error, false);
                }
            } else {
                endMove(GripperMoveResult::Stopped, false);
            }
            return;

        case MoveStep::OpenResetPause:
            return;

        case MoveStep::ResetOpen:
            if (axisResult == AxisMoveResult::Stalled) {
                publishMoveEvent(CmdType::OPEN_RESET, GripperMoveResult::Stalled);
                if (!startMoveStep(MoveStep::ResetForward, ParameterConfig::GRIPPER_RESET_FORWARD_STEPS, false)) {
                    endMove(GripperMoveResult::Error, false);
                }
            } else if (axisResult == AxisMoveResult::Done) {
                publishMoveEvent(CmdType::OPEN_RESET, GripperMoveResult::Done);
                mMoveStep = MoveStep::Idle;
                mActiveCommand = CmdType::NONE;
                mSilentReset = false;
                mIsBusy = false;
                mLastResult = GripperMoveResult::Error;
                mAxis.setEnabled(false);
            } else {
                publishMoveEvent(CmdType::OPEN_RESET, GripperMoveResult::Stopped);
                mMoveStep = MoveStep::Idle;
                mActiveCommand = CmdType::NONE;
                mSilentReset = false;
                mIsBusy = false;
                mLastResult = GripperMoveResult::Stopped;
                mAxis.setEnabled(false);
            }
            return;

        case MoveStep::ResetForward:
            if (axisResult == AxisMoveResult::Done) {
                endMove(GripperMoveResult::Done, false);
            } else if (axisResult == AxisMoveResult::Stalled) {
                endMove(GripperMoveResult::Stalled, false);
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
        mSilentReset = false;
        mIsBusy = false;
        return false;
    }

    if (!mAxis.move(steps, stopOnStall)) {
        mMoveStep = MoveStep::Idle;
        mActiveCommand = CmdType::NONE;
        mSilentReset = false;
        mIsBusy = false;
        return false;
    }

    return true;
}

void Gripper::startOpenResetPause() {
    mAxis.setEnabled(false);
    mMoveStep = MoveStep::OpenResetPause;
    mPauseUntil = make_timeout_time_ms(2000);
    mSilentReset = true;
}

bool Gripper::startResetSequence(bool silent) {
    mSilentReset = silent;
    return startMoveStep(MoveStep::ResetOpen, -ParameterConfig::GRIPPER_RESET_OPEN_STEPS, true);
}

void Gripper::endMove(GripperMoveResult result, bool keepMotorEnabled) {
    const CmdType completedCommand = mActiveCommand;
    const CmdType completedMoveCommand = moveStepCmd(mMoveStep);

    mMoveStep = MoveStep::Idle;
    mActiveCommand = CmdType::NONE;
    mSilentReset = false;
    mIsBusy = false;
    mLastResult = result;
    if (completedCommand == CmdType::RESET) {
        publishMoveEvent(CmdType::RESET, result);
    } else if (completedCommand == CmdType::OPEN || completedCommand == CmdType::CLOSE) {
        publishMoveEvent(completedMoveCommand, result);
    }

    if (!keepMotorEnabled) {
        mAxis.setEnabled(false);
    }
}

CmdType Gripper::moveStepCmd(MoveStep moveStep) const {
    switch (moveStep) {
        case MoveStep::Close:
            return CmdType::CLOSE;
        case MoveStep::Open:
            return CmdType::OPEN;
        case MoveStep::ResetOpen:
            return CmdType::OPEN_RESET;
        case MoveStep::ResetForward:
            return CmdType::OPEN_RESET_FORWARD;
        case MoveStep::OpenResetPause:
        case MoveStep::Idle:
            return mActiveCommand;
    }

    return mActiveCommand;
}

void Gripper::publishMoveEvent(CmdType cmd, GripperMoveResult result, EventType eventType) {
    mMoveEvent.cmd = cmd;
    mMoveEvent.eventType = eventType;
    mMoveEvent.result = result;
    mHasMoveEvent = true;
}
