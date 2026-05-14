#include "stepper/StepperAxis.h"
#include "config/ParameterConfig.h"
#include "interface/TestInterface.h"
#include "stepper/TMC2209Driver.h"
#include <cmath>
#include "hardware/gpio.h"

StepperAxis* StepperAxis::mDiagOwners[StepperAxis::MaxGpioCount] = {};

StepperAxis::StepperAxis(TMC2209Driver& driver)
    : mDriver(driver) {}

void StepperAxis::setup() {
    gpio_init(PinConfig::DIR_PIN);
    gpio_set_dir(PinConfig::DIR_PIN, GPIO_OUT);
    gpio_put(PinConfig::DIR_PIN, 0);

    gpio_init(PinConfig::EN_PIN);
    gpio_set_dir(PinConfig::EN_PIN, GPIO_OUT);
    gpio_put(PinConfig::EN_PIN, 1);
    
    mPWM.setup(PinConfig::STEP_PIN);
    mPWM.setFrequency(ParameterConfig::AXIS_START_SPEED_SPS);

    if (PinConfig::STALL_DETECTION_MODE == PinConfig::StallDetectionMode::DiagInterrupt) {
        configureStallInterrupt();
    } else {
        disableStallInterrupt();
    }
}

bool StepperAxis::move(int32_t steps, bool stopOnStall) {
    if (mIsBusy || steps == 0) {
        return false;
    }

    setEnabled(true);
    gpio_put(PinConfig::DIR_PIN, (steps > 0) ? 1 : 0);

    mRemainingSteps = (steps > 0) ? steps : -steps;
    mStopOnStall = stopOnStall;
    mDiagStallLatched = false;
    mLastMoveResult = AxisMoveResult::None;
    mExecutedSteps = 0;
    mStallDetectionArmedAt = make_timeout_time_us(ParameterConfig::DRIVER_STALL_ARM_DELAY_US);
    mNextUartStallPollAt = get_absolute_time();
    mConsecutiveUartStallSamples = 0;
    mPeakStallGuardResult = 0;
    mStallGuardPrimed = false;
    mFilteredStallGuardResult = 0;
    mHasFilteredStallGuardResult = false;
    mCurrentStepFrequencyHz = static_cast<float>(ParameterConfig::AXIS_START_SPEED_SPS);

    mPWM.setFrequency(ParameterConfig::AXIS_START_SPEED_SPS);
    mIsBusy = true;

    mPWM.start();
    return true;
}

void StepperAxis::stop() {
    if (!mIsBusy) {
        return;
    }

    endMove(AxisMoveResult::Stopped);
}

void StepperAxis::update() {
    if (!mIsBusy) {
        return;
    }

    const bool stalled = checkStall();

    // Keep polling SG_RESULT during motion. Only stop when stall-stop is enabled.
    if (mStopOnStall && stalled) {
        endMove(AxisMoveResult::Stalled);
        return;
    }

    const uint32_t wrapCount = mPWM.takeWrapCount();
    if (wrapCount == 0) {
        return;
    }

    const uint32_t completedSteps =
        (wrapCount < static_cast<uint32_t>(mRemainingSteps)) ? wrapCount : static_cast<uint32_t>(mRemainingSteps);
    mRemainingSteps -= static_cast<int32_t>(completedSteps);
    mExecutedSteps += completedSteps;

    if (mRemainingSteps <= 0) {
        endMove(AxisMoveResult::Done);
        return;
    }

    updateMotionSpeed();
}

void StepperAxis::setEnabled(bool enabled) {
    if (!enabled && mIsBusy) {
        endMove(AxisMoveResult::Stopped);
    }

    if (enabled) {
        gpio_put(PinConfig::EN_PIN, 0);
    } else {
        gpio_put(PinConfig::EN_PIN, 1);
    }
}

bool StepperAxis::isBusy() const {
    return mIsBusy;
}

AxisMoveResult StepperAxis::getLastMoveResult() const {
    return mLastMoveResult;
}

void StepperAxis::configureStallInterrupt() {
    if (PinConfig::DIAG_PIN >= MaxGpioCount) {
        return;
    }

    if (mStallInterruptConfigured) {
        disableStallInterrupt();
    }

    gpio_init(PinConfig::DIAG_PIN);
    gpio_set_dir(PinConfig::DIAG_PIN, GPIO_IN);
    gpio_disable_pulls(PinConfig::DIAG_PIN);

    mDiagOwners[PinConfig::DIAG_PIN] = this;
    mStallInterruptConfigured = true;
    mDiagStallLatched = false;

    gpio_set_irq_enabled_with_callback(PinConfig::DIAG_PIN, PinConfig::DIAG_IRQ_EVENT_MASK, true, &StepperAxis::stallIrqRouter);
}

void StepperAxis::disableStallInterrupt() {
    if (!mStallInterruptConfigured || PinConfig::DIAG_PIN >= MaxGpioCount) {
        return;
    }

    gpio_set_irq_enabled(PinConfig::DIAG_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL | GPIO_IRQ_LEVEL_LOW | GPIO_IRQ_LEVEL_HIGH, false);

    if (mDiagOwners[PinConfig::DIAG_PIN] == this) {
        mDiagOwners[PinConfig::DIAG_PIN] = nullptr;
    }

    mStallInterruptConfigured = false;
    mDiagStallLatched = false;
}

bool StepperAxis::checkStall() {
    // Check for a motor stall using the configured detection method.
    // Depending on the mode, this reads the driver over UART, uses the
    // DIAG interrupt, or always reports no stall.
    switch (PinConfig::STALL_DETECTION_MODE) {
        case PinConfig::StallDetectionMode::None:
            return false;

        case PinConfig::StallDetectionMode::Uart: {
            if (absolute_time_diff_us(get_absolute_time(), mNextUartStallPollAt) > 0) {
                return false;
            }

            mNextUartStallPollAt = make_timeout_time_us(ParameterConfig::DRIVER_STALL_POLL_INTERVAL_US);
            uint16_t sgResult = 0;
            uint8_t threshold = 0;
            bool triggered = false;
            if (!mDriver.readStallGuardStatus(sgResult, threshold, triggered)) {
                mConsecutiveUartStallSamples = 0;
                return false;
            }
            (void)triggered;

            const uint16_t compareValue = static_cast<uint16_t>(threshold) * 2u;

            if (!mHasFilteredStallGuardResult) {
                mFilteredStallGuardResult = sgResult;
                mHasFilteredStallGuardResult = true;
            } else {
                mFilteredStallGuardResult = static_cast<uint16_t>((mFilteredStallGuardResult + sgResult + 1u) / 2u);
            }

            TestInterface::logf("%u\r\n", static_cast<unsigned>(mFilteredStallGuardResult));

            updateStallGuardPriming(mFilteredStallGuardResult, compareValue);
            const bool detectionActive = isStallDetectionActive();
            if (!detectionActive) {
                mConsecutiveUartStallSamples = 0;
                return false;
            }

            const bool filteredStallDetected = mFilteredStallGuardResult <= compareValue;
            if (filteredStallDetected) {
                if (mConsecutiveUartStallSamples < 0xFFu) {
                    ++mConsecutiveUartStallSamples;
                }
            } else {
                mConsecutiveUartStallSamples = 0;
            }

            const bool stopTriggered =
                mConsecutiveUartStallSamples >= ParameterConfig::DRIVER_STALL_CONSECUTIVE_SAMPLES;
            return stopTriggered;
        }

        case PinConfig::StallDetectionMode::DiagInterrupt: {
            if (!mDiagStallLatched) {
                return false;
            }

            if (!isBasicStallWindowActive() || isInBrakingZone()) {
                mDiagStallLatched = false;
                return false;
            }

            return true;
        }
    }
    return false;
}

void StepperAxis::endMove(AxisMoveResult result) {
    mIsBusy = false;
    mRemainingSteps = 0;
    mDiagStallLatched = false;
    mStopOnStall = false;
    mConsecutiveUartStallSamples = 0;
    mPeakStallGuardResult = 0;
    mStallGuardPrimed = false;
    mFilteredStallGuardResult = 0;
    mHasFilteredStallGuardResult = false;
    mLastMoveResult = result;
    mPWM.stop();
}

bool StepperAxis::isBasicStallWindowActive() const {
    const int64_t armDelayRemainingUs = absolute_time_diff_us(get_absolute_time(), mStallDetectionArmedAt);
    if (armDelayRemainingUs > 0) {
        return false;
    }

    return mExecutedSteps >= ParameterConfig::DRIVER_STALL_ARM_STEPS;
}

bool StepperAxis::isInBrakingZone() const {
    const float startSpeedHz = static_cast<float>(ParameterConfig::AXIS_START_SPEED_SPS);
    const float accelerationSps2 = static_cast<float>(ParameterConfig::AXIS_ACCELERATION_SPS2);
    if (accelerationSps2 <= 0.0f) {
        return false;
    }

    const float currentSpeedSquared = mCurrentStepFrequencyHz * mCurrentStepFrequencyHz;
    const float startSpeedSquared = startSpeedHz * startSpeedHz;
    const float stepsToBrake =
        (currentSpeedSquared > startSpeedSquared)
            ? (currentSpeedSquared - startSpeedSquared) / (2.0f * accelerationSps2)
            : 0.0f;
    const float brakingWindowSteps =
        stepsToBrake + static_cast<float>(ParameterConfig::DRIVER_STALL_BRAKE_MARGIN_STEPS);
    return static_cast<float>(mRemainingSteps) <= brakingWindowSteps;
}

void StepperAxis::updateStallGuardPriming(uint16_t sgResult, uint16_t compareValue) {
    if (sgResult > mPeakStallGuardResult) {
        mPeakStallGuardResult = sgResult;
        return;
    }

    if (mStallGuardPrimed) {
        return;
    }

    uint16_t primingFloor = ParameterConfig::DRIVER_STALL_PRIME_MIN_SG_RESULT;
    const uint16_t compareFloor = static_cast<uint16_t>(compareValue + ParameterConfig::DRIVER_STALL_PRIME_COMPARE_MARGIN);
    if (compareFloor > primingFloor) {
        primingFloor = compareFloor;
    }

    if (mPeakStallGuardResult < primingFloor) {
        return;
    }

    if (sgResult + ParameterConfig::DRIVER_STALL_PRIME_DROP_DELTA <= mPeakStallGuardResult) {
        mStallGuardPrimed = true;
    }
}

bool StepperAxis::isStallDetectionActive() const {
    if (!isBasicStallWindowActive()) {
        return false;
    }

    if (!mStallGuardPrimed) {
        return false;
    }

    return true;
}

void StepperAxis::updateMotionSpeed() {
    const float startSpeedHz = static_cast<float>(ParameterConfig::AXIS_START_SPEED_SPS);
    const float targetSpeedHz = static_cast<float>(ParameterConfig::AXIS_TARGET_SPEED_SPS);
    const float accelerationSps2 = static_cast<float>(ParameterConfig::AXIS_ACCELERATION_SPS2);

    if (accelerationSps2 <= 0.0f) {
        mCurrentStepFrequencyHz = targetSpeedHz;
        mPWM.setFrequency(mCurrentStepFrequencyHz);
        return;
    }

    const float startSpeedSquared = startSpeedHz * startSpeedHz;
    const float targetSpeedSquared = targetSpeedHz * targetSpeedHz;
    const float accelerateSpeedSquared =
        startSpeedSquared + (2.0f * accelerationSps2 * static_cast<float>(mExecutedSteps));
    float speedSquared = accelerateSpeedSquared;
    if (targetSpeedSquared < speedSquared) {
        speedSquared = targetSpeedSquared;
    }
    if (speedSquared < startSpeedSquared) {
        speedSquared = startSpeedSquared;
    }

    mCurrentStepFrequencyHz = std::sqrt(speedSquared);

    mPWM.setFrequency(mCurrentStepFrequencyHz);
}

void StepperAxis::stallIrqRouter(uint gpio, uint32_t events) {
    // GPIO interrupts are plain C callbacks, so they cannot call a specific
    // StepperAxis object directly. This router looks up which axis owns the
    // DIAG pin that fired and forwards the interrupt to that object.
    if (gpio >= MaxGpioCount) {
        return;
    }

    StepperAxis* owner = mDiagOwners[gpio];
    if (owner != nullptr) {
        owner->handleStallInterrupt(gpio, events);
    }
}

void StepperAxis::handleStallInterrupt(uint gpio, uint32_t events) {
    (void)gpio;     // to avoid unused parameter warning.
    (void)events;   // to avoid unused parameter warning.

    mDiagStallLatched = true;
}
