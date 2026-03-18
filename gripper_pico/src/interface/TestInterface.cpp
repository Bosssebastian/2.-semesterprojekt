#include "TestInterface.h"

#include <stdio.h>

#include "pico/stdlib.h"

TestInterface::TestInterface(TMC2209Driver& driver, StepperAxis& axis, Gripper& gripper)
    : mDriver(driver),
      mAxis(axis),
      mGripper(gripper),
      mPreviousGripperBusy(gripper.isBusy()) {}

void TestInterface::begin(bool driverConfigured) {
    mDriverConfigured = driverConfigured;
    sleep_ms(UsbStartupDelayMs);
}

void TestInterface::update() {
    handleUsbConsole();
    updateAutoCycle();

    if (!mConsoleActive) {
        mPreviousGripperBusy = mGripper.isBusy();
        return;
    }

    const bool currentGripperBusy = mGripper.isBusy();
    if (mPreviousGripperBusy && !currentGripperBusy) {
        printf("Move complete: gripper=%s axis=%s\n",
               toString(mGripper.getLastResult()),
               toString(mAxis.getLastMoveResult()));

        if (mAutoCycle.enabled) {
            scheduleAutoCycle(AutoCyclePauseMs);
        }
    }
    mPreviousGripperBusy = currentGripperBusy;
}

const char* TestInterface::toString(AxisMoveResult result) {
    switch (result) {
        case AxisMoveResult::None: return "none";
        case AxisMoveResult::Done: return "done";
        case AxisMoveResult::Stalled: return "stalled";
        case AxisMoveResult::Stopped: return "stopped";
    }

    return "unknown";
}

const char* TestInterface::toString(GripperResult result) {
    switch (result) {
        case GripperResult::None: return "none";
        case GripperResult::Done: return "done";
        case GripperResult::Stalled: return "stalled";
        case GripperResult::Stopped: return "stopped";
        case GripperResult::Error: return "error";
    }

    return "unknown";
}

void TestInterface::activateConsole() {
    if (mConsoleActive) {
        return;
    }

    mConsoleActive = true;
    printf("\nGripper Pico test interface\n");
    printf("Driver configuration: %s\n", mDriverConfigured ? "ok" : "failed");
    printHelp();
}

void TestInterface::printHelp() const {
    printf("\nUSB test console commands:\n");
    printf("  h : show this help\n");
    printf("  i : print axis/gripper status\n");
    printf("  t : run driver connection test\n");
    printf("  d : read driver diagnostics\n");
    printf("  c : close gripper\n");
    printf("  o : open gripper\n");
    printf("  s : stop active move\n");
    printf("  e : enable motor output\n");
    printf("  x : disable motor output\n");
    printf("  a : toggle automatic close/open cycle\n\n");
}

void TestInterface::printStatus() const {
    printf("Status: axisBusy=%s axisResult=%s gripperBusy=%s gripperResult=%s autoCycle=%s\n",
           mAxis.isBusy() ? "yes" : "no",
           toString(mAxis.getLastMoveResult()),
           mGripper.isBusy() ? "yes" : "no",
           toString(mGripper.getLastResult()),
           mAutoCycle.enabled ? "on" : "off");
}

void TestInterface::testDriverConnection() const {
    if (mDriver.testConnection()) {
        printf("Driver connection: ok\n");
    } else {
        printf("Driver connection: failed\n");
    }
}


void TestInterface::printDriverDiagnostics() const {
    testDriverConnection();

    uint8_t writeCounter = 0;
    if (mDriver.getWriteCounter(writeCounter)) {
        printf("IFCNT: %u\n", writeCounter);
    } else {
        printf("IFCNT: read failed\n");
    }

    uint32_t driverStatus = 0;
    if (mDriver.readDriverStatus(driverStatus)) {
        printf("DRV_STATUS: 0x%08lx\n", static_cast<unsigned long>(driverStatus));
    } else {
        printf("DRV_STATUS: read failed\n");
    }

    bool diagActive = false;
    if (mDriver.readDiagState(diagActive)) {
        printf("DIAG: %s\n", diagActive ? "active" : "inactive");
    } else {
        printf("DIAG: read failed\n");
    }

    uint16_t stallGuardResult = 0;
    if (mDriver.readStallGuardResult(stallGuardResult)) {
        printf("SG_RESULT: %u\n", stallGuardResult);
    } else {
        printf("SG_RESULT: read failed\n");
    }
}

void TestInterface::scheduleAutoCycle(uint32_t delayMs) {
    mAutoCycle.actionScheduled = true;
    mAutoCycle.nextActionAt = make_timeout_time_ms(delayMs);
}

void TestInterface::handleUsbConsole() {
    const int input = getchar_timeout_us(0);
    if (input == PICO_ERROR_TIMEOUT) {
        return;
    }

    if (!mConsoleActive) {
        if (input == '\r' || input == '\n') {
            return;
        }

        activateConsole();
    }

    handleConsoleCommand(input);
}

void TestInterface::handleConsoleCommand(int input) {
    switch (input) {
        case '\r':
        case '\n':
            return;

        case 'h':
        case '?':
            printHelp();
            return;

        case 'i':
            printStatus();
            return;

        case 't':
            testDriverConnection();
            return;

        case 'd':
            printDriverDiagnostics();
            return;

        case 'c':
            mAutoCycle.enabled = false;
            mAutoCycle.actionScheduled = false;
            printf(mGripper.close(true) ? "Close started\n" : "Close rejected\n");
            return;

        case 'o':
            mAutoCycle.enabled = false;
            mAutoCycle.actionScheduled = false;
            printf(mGripper.open(true) ? "Open started\n" : "Open rejected\n");
            return;

        case 's':
            mAutoCycle.enabled = false;
            mAutoCycle.actionScheduled = false;
            mGripper.stop();
            printf("Stop requested\n");
            return;

        case 'e':
            mAxis.setEnabled(true);
            printf("Motor enabled\n");
            return;

        case 'x':
            mAutoCycle.enabled = false;
            mAutoCycle.actionScheduled = false;
            mAxis.setEnabled(false);
            printf("Motor disabled\n");
            return;

        case 'a':
            mAutoCycle.enabled = !mAutoCycle.enabled;
            mAutoCycle.nextCommandIsClose = true;
            if (mAutoCycle.enabled) {
                scheduleAutoCycle(0);
                printf("Automatic cycle enabled\n");
            } else {
                mAutoCycle.actionScheduled = false;
                printf("Automatic cycle disabled\n");
            }
            return;

        default:
            printf("Unknown command: %c\n", static_cast<char>(input));
            return;
    }
}

void TestInterface::updateAutoCycle() {
    if (!mAutoCycle.enabled || !mAutoCycle.actionScheduled || mGripper.isBusy()) {
        return;
    }

    if (absolute_time_diff_us(get_absolute_time(), mAutoCycle.nextActionAt) > 0) {
        return;
    }

    const bool started = mAutoCycle.nextCommandIsClose ? mGripper.close(true) : mGripper.open(true);
    if (!started) {
        mAutoCycle.enabled = false;
        mAutoCycle.actionScheduled = false;
        if (mConsoleActive) {
            printf("Automatic cycle aborted\n");
        }
        return;
    }

    if (mConsoleActive) {
        printf("Automatic cycle: %s\n", mAutoCycle.nextCommandIsClose ? "close" : "open");
    }

    mAutoCycle.nextCommandIsClose = !mAutoCycle.nextCommandIsClose;
    mAutoCycle.actionScheduled = false;
}
