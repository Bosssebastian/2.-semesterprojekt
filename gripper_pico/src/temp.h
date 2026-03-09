#pragma once

#include "Gripper.h"
#include "StepperAxis.h"
#include "TMC2209Driver.h"

#include "pico/time.h"

class TempHarness {
public:
    TempHarness(TMC2209Driver& driver, StepperAxis& axis, Gripper& gripper);

    void begin(bool driverConfigured);
    void update();

private:
    static constexpr uint32_t UsbStartupDelayMs = 2000;
    static constexpr uint32_t AutoCyclePauseMs = 1000;

    struct AutoCycleState {
        bool enabled = false;
        bool actionScheduled = false;
        bool nextCommandIsClose = true;
        absolute_time_t nextActionAt = {};
    };

    TMC2209Driver& mDriver;
    StepperAxis& mAxis;
    Gripper& mGripper;
    AutoCycleState mAutoCycle;
    bool mPreviousGripperBusy = false;

    static const char* toString(AxisMoveResult result);
    static const char* toString(GripperResult result);

    void printHelp() const;
    void printStatus() const;
    void printDriverDiagnostics() const;
    void scheduleAutoCycle(uint32_t delayMs);
    void handleUsbConsole();
    void updateAutoCycle();
};
