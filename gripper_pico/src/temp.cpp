#include "temp.h"

#include <stdio.h>

#include "TMC2209Registers.h"
#include "pico/stdlib.h"

TempHarness::TempHarness(TMC2209Driver& driver, StepperAxis& axis, Gripper& gripper)
    : mDriver(driver),
      mAxis(axis),
      mGripper(gripper),
      mPreviousGripperBusy(gripper.isBusy()) {}

void TempHarness::begin(bool driverConfigured) {
    sleep_ms(UsbStartupDelayMs);

    printf("\nGripper Pico test harness\n");
    printf("Driver configuration: %s\n", driverConfigured ? "ok" : "failed");
    printHelp();
    printDriverDiagnostics();
}

void TempHarness::update() {
    handleUsbConsole();
    updateAutoCycle();

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

const char* TempHarness::toString(AxisMoveResult result) {
    switch (result) {
        case AxisMoveResult::None: return "none";
        case AxisMoveResult::Done: return "done";
        case AxisMoveResult::Stalled: return "stalled";
        case AxisMoveResult::Stopped: return "stopped";
    }

    return "unknown";
}

const char* TempHarness::toString(GripperResult result) {
    switch (result) {
        case GripperResult::None: return "none";
        case GripperResult::Done: return "done";
        case GripperResult::Stalled: return "stalled";
        case GripperResult::Stopped: return "stopped";
        case GripperResult::Error: return "error";
    }

    return "unknown";
}

void TempHarness::printHelp() const {
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

void TempHarness::printStatus() const {
    printf("Status: axisBusy=%s axisResult=%s gripperBusy=%s gripperResult=%s autoCycle=%s\n",
           mAxis.isBusy() ? "yes" : "no",
           toString(mAxis.getLastMoveResult()),
           mGripper.isBusy() ? "yes" : "no",
           toString(mGripper.getLastResult()),
           mAutoCycle.enabled ? "on" : "off");
}

void TempHarness::testDriverConnection() const {
    printf("Driver connection test:\n");
    printf("  Note: write status only confirms the frame was sent by the Pico.\n");
    printf("  A real UART link is only confirmed once IFCNT or a register read succeeds.\n");

    uint8_t writeCounterBefore = 0;
    uint8_t writeCounterAfter = 0;
    uint32_t gconf = 0;

    bool primedUart = false;
    if (!mDriver.getWriteCounter(writeCounterBefore)) {
        printf("  IFCNT read before: failed\n");

        const uint32_t uartEnableGconf =
            TMC2209Bits::GCONF::I_SCALE_ANALOG |
            TMC2209Bits::GCONF::PDN_DISABLE |
            TMC2209Bits::GCONF::MULTISTEP_FILT;

        const bool primeWriteOk = mDriver.writeRegister(TMC2209Reg::GCONF, uartEnableGconf);
        printf("  GCONF prime write frame: %s\n", primeWriteOk ? "sent" : "failed");
        if (!primeWriteOk) {
            printf("  Result: failed\n");
            return;
        }

        primedUart = true;
        if (!mDriver.getWriteCounter(writeCounterBefore)) {
            printf("  IFCNT read after prime: failed\n");
            printf("  Likely cause: TMC2209 is not replying on RX.\n");
            printf("  Check PDN_UART wiring, TX/RX coupling for single-wire UART, common GND, and address/MS pins.\n");
            printf("  Result: failed\n");
            return;
        }
    }

    printf("  IFCNT before: %u%s\n", writeCounterBefore, primedUart ? " (after prime)" : "");

    if (!mDriver.readRegister(TMC2209Reg::GCONF, gconf)) {
        printf("  GCONF read: failed\n");
        printf("  Likely cause: write-only TX path works, but the read reply is not reaching the Pico RX pin.\n");
        printf("  Result: failed\n");
        return;
    }

    printf("  GCONF read: ok (0x%08lx)\n", static_cast<unsigned long>(gconf));

    if (!mDriver.writeRegister(TMC2209Reg::GCONF, gconf)) {
        printf("  GCONF write-back: failed\n");
        printf("  Result: failed\n");
        return;
    }

    printf("  GCONF write-back frame: sent\n");

    if (!mDriver.getWriteCounter(writeCounterAfter)) {
        printf("  IFCNT read after write: failed\n");
        printf("  Result: failed\n");
        return;
    }

    printf("  IFCNT after: %u\n", writeCounterAfter);

    const bool counterIncrementOk = static_cast<uint8_t>(writeCounterBefore + 1u) == writeCounterAfter;
    printf("  IFCNT increment: %s\n", counterIncrementOk ? "ok" : "failed");
    printf("  Result: %s\n", counterIncrementOk ? "ok" : "failed");
}

void TempHarness::printDriverDiagnostics() const {
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

void TempHarness::scheduleAutoCycle(uint32_t delayMs) {
    mAutoCycle.actionScheduled = true;
    mAutoCycle.nextActionAt = make_timeout_time_ms(delayMs);
}

void TempHarness::handleUsbConsole() {
    const int input = getchar_timeout_us(0);
    if (input == PICO_ERROR_TIMEOUT) {
        return;
    }

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
            printf(mGripper.close() ? "Close started\n" : "Close rejected\n");
            return;

        case 'o':
            mAutoCycle.enabled = false;
            mAutoCycle.actionScheduled = false;
            printf(mGripper.open() ? "Open started\n" : "Open rejected\n");
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

void TempHarness::updateAutoCycle() {
    if (!mAutoCycle.enabled || !mAutoCycle.actionScheduled || mGripper.isBusy()) {
        return;
    }

    if (absolute_time_diff_us(get_absolute_time(), mAutoCycle.nextActionAt) > 0) {
        return;
    }

    const bool started = mAutoCycle.nextCommandIsClose ? mGripper.close() : mGripper.open();
    if (!started) {
        mAutoCycle.enabled = false;
        mAutoCycle.actionScheduled = false;
        printf("Automatic cycle aborted\n");
        return;
    }

    printf("Automatic cycle: %s\n", mAutoCycle.nextCommandIsClose ? "close" : "open");
    mAutoCycle.nextCommandIsClose = !mAutoCycle.nextCommandIsClose;
    mAutoCycle.actionScheduled = false;
}
