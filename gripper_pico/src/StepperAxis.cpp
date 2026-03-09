#include "StepperAxis.h"
#include "ParameterConfig.h"
#include "TMC2209Driver.h"
#include <cmath>
#include "hardware/gpio.h"

StepperAxis* StepperAxis::mDiagOwners[StepperAxis::MaxGpioCount] = {};

StepperAxis::StepperAxis(TMC2209Driver* driver): mDriver(driver) {}

void StepperAxis::begin() {
    gpio_init(PinConfig::DIR_PIN);
    gpio_set_dir(PinConfig::DIR_PIN, GPIO_OUT);
    gpio_put(PinConfig::DIR_PIN, 0);

    gpio_init(PinConfig::EN_PIN);
    gpio_set_dir(PinConfig::EN_PIN, GPIO_OUT);
    gpio_put(PinConfig::EN_PIN, 1);
    
    mPWM.begin(PinConfig::STEP_PIN);
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

    // Stop movement if stall is detected
    if (mStopOnStall && checkStall()) {
        endMove(AxisMoveResult::Stalled);
        return;
    }

    // The PWM counter can finish several cycles before update() runs again.
    // Each finished cycle is counted by the IRQ and treated here as one step.
    const uint32_t wrapCount = mPWM.takeWrapCount();
    for (uint32_t wrapIndex = 0; wrapIndex < wrapCount; ++wrapIndex) {
        if (mRemainingSteps > 0) {
            --mRemainingSteps;
        }

        if (mRemainingSteps <= 0) {
            endMove(AxisMoveResult::Done);
            return;
        }

        updateMotionSpeed();
    }
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
            bool stallDetected = false;
            if (!mDriver->isStallGuardTriggered(stallDetected)) {
                return false;
            }
            return stallDetected;
        }

        case PinConfig::StallDetectionMode::DiagInterrupt:
            return mDiagStallLatched;
    }
    return false;
}

void StepperAxis::endMove(AxisMoveResult result) {
    mIsBusy = false;
    mRemainingSteps = 0;
    mDiagStallLatched = false;
    mStopOnStall = false;
    mLastMoveResult = result;
    mPWM.stop();
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

    const float currentSpeedSquared = mCurrentStepFrequencyHz * mCurrentStepFrequencyHz;
    const float startSpeedSquared = startSpeedHz * startSpeedHz;
    const float stepsToBrake = (currentSpeedSquared > startSpeedSquared) ? (currentSpeedSquared - startSpeedSquared) / (2.0f * accelerationSps2) : 0.0f;

    if (static_cast<float>(mRemainingSteps) <= stepsToBrake) {
        const float nextSpeedSquared = currentSpeedSquared - (2.0f * accelerationSps2);
        mCurrentStepFrequencyHz = std::sqrt((nextSpeedSquared > startSpeedSquared) ? nextSpeedSquared : startSpeedSquared);
    } else if (mCurrentStepFrequencyHz < targetSpeedHz) {
        const float targetSpeedSquared = targetSpeedHz * targetSpeedHz;
        const float nextSpeedSquared = currentSpeedSquared + (2.0f * accelerationSps2);
        mCurrentStepFrequencyHz = std::sqrt((nextSpeedSquared < targetSpeedSquared) ? nextSpeedSquared : targetSpeedSquared);
    } else {
        mCurrentStepFrequencyHz = targetSpeedHz;
    }

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
