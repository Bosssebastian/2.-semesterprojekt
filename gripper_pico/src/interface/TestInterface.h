#pragma once

#include "Gripper.h"
#include "stepper/StepperAxis.h"

#include "pico/time.h"

class TestInterface {
public:
    explicit TestInterface(Gripper& gripper);

    void setup(bool driverConfigured);
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

    Gripper& mGripper;
    AutoCycleState mAutoCycle;
    bool mPreviousGripperBusy = false;
    bool mConsoleActive = false;
    bool mDriverConfigured = false;

    static const char* toString(AxisMoveResult result);
    static const char* toString(GripperResult result);

    void activateConsole();
    void printHelp() const;
    void printStatus() const;
    void testDriverConnection() const;
    void printDriverDiagnostics() const;
    void scheduleAutoCycle(uint32_t delayMs);
    void handleUsbConsole();
    void handleConsoleCommand(int input);
    void updateAutoCycle();
};
