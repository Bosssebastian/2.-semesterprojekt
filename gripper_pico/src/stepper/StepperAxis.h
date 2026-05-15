#pragma once
#include <cstdint>
#include "config/PinConfig.h"
#include "pico/time.h"
#include "stepper/PwmOutput.h"
#include "pico/types.h"
#include "CurrentSensor.h"
#include "stepper/TMC2209Driver.h"

enum class AxisMoveResult : uint8_t {
    None,
    Done,
    Stalled,
    Stopped
};

inline const char* toString(AxisMoveResult result) {
    switch (result) {
        case AxisMoveResult::None:
            return "None";
        case AxisMoveResult::Done:
            return "Done";
        case AxisMoveResult::Stalled:
            return "Stalled";
        case AxisMoveResult::Stopped:
            return "Stopped";
    }

    return "Unknown";
}

class StepperAxis {
public:
    StepperAxis(TMC2209Driver& driver, CurrentSensor& currentSensor);
    void setup();
    void update();

    bool move(int32_t steps, bool stopOnStall);
    void stop();
    void setEnabled(bool enabled);
    void setStallValueEventsEnabled(bool enabled);
    bool stallValueEventsEnabled() const;

    bool isBusy() const;
    AxisMoveResult getLastMoveResult() const;

private:
    PwmOutput mPWM;
    TMC2209Driver& mDriver;
    CurrentSensor& mCurrentSensor;

    volatile bool mIsBusy = false;
    bool mStopOnStall = false;
    bool mStallInterruptConfigured = false;
    bool mStallValueEventsEnabled = false;

    volatile bool mDiagStallLatched = false;
    volatile int32_t mRemainingSteps = 0;
    uint32_t mExecutedSteps = 0;
    absolute_time_t mStallDetectionArmedAt = {};
    absolute_time_t mNextUartStallPollAt = {};
    uint8_t mConsecutiveUartStallSamples = 0;
    uint8_t mConsecutiveCurrentStallSamples = 0;
    uint32_t mLastCurrentStallSampleSequence = 0;
    uint16_t mPeakStallGuardResult = 0;
    bool mStallGuardPrimed = false;
    uint16_t mFilteredStallGuardResult = 0;
    bool mHasFilteredStallGuardResult = false;

    float mCurrentStepFrequencyHz = 0.0f;
    AxisMoveResult mLastMoveResult = AxisMoveResult::None;

    void configureStallInterrupt();
    void disableStallInterrupt();
    bool checkStall();
    void endMove(AxisMoveResult result);
    bool isBasicStallWindowActive() const;
    bool isInBrakingZone() const;
    void updateStallGuardPriming(uint16_t sgResult, uint16_t compareValue);
    bool isStallDetectionActive() const;
    bool isCurrentStallDetectionActive() const;
    void sendStallValueEvent(const char* mode, float value) const;
    void sendStallValueEvent(const char* mode, uint16_t value) const;
    void updateMotionSpeed();
    static void stallIrqRouter(uint gpio, uint32_t events);
    void handleStallInterrupt(uint gpio, uint32_t events);

    static constexpr uint MaxGpioCount = 32;
    static StepperAxis* mDiagOwners[MaxGpioCount];
};
