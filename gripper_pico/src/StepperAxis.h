#pragma once
#include <cstdint>
#include "PinConfig.h"
#include "PwmOutput.h"
#include "pico/types.h"
#include "TMC2209Driver.h"

enum class AxisMoveResult : uint8_t {
    None,
    Done,
    Stalled,
    Stopped
};

class StepperAxis {
public:
    explicit StepperAxis(TMC2209Driver* driver);

    void begin();
    bool move(int32_t steps, bool stopOnStall);
    void stop();
    void update();
    void setEnabled(bool enabled);

    bool isBusy() const;
    AxisMoveResult getLastMoveResult() const;

private:
    PwmOutput mPWM;
    TMC2209Driver* mDriver = nullptr;

    volatile bool mIsBusy = false;
    bool mStopOnStall = false;
    bool mStallInterruptConfigured = false;

    volatile bool mDiagStallLatched = false;
    volatile int32_t mRemainingSteps = 0;

    float mCurrentStepFrequencyHz = 0.0f;
    AxisMoveResult mLastMoveResult = AxisMoveResult::None;

    void configureStallInterrupt();
    void disableStallInterrupt();
    bool checkStall();
    void endMove(AxisMoveResult result);
    void updateMotionSpeed();
    static void stallIrqRouter(uint gpio, uint32_t events);
    void handleStallInterrupt(uint gpio, uint32_t events);

    static constexpr uint MaxGpioCount = 32;
    static StepperAxis* mDiagOwners[MaxGpioCount];
};
