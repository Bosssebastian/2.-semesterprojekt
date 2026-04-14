#include "TestInterface.h"
#include "../../../shared/Types.h"
#include "stepper/TMC2209Driver.h"
#include "stepper/TMC2209Registers.h"
#include <cstdarg>
#include <cstdio>
#include <queue>
#include <string>
#include "pico/stdlib.h"

bool TestInterface::sStallDebugEnabled = false;

void TestInterface::setup() {
    stdio_init_all();
    sleep_ms(UsbStartupDelayMs);
}

void TestInterface::update() {
    const int input = getchar_timeout_us(0);
    if (input == PICO_ERROR_TIMEOUT) {
        return;
    }

    if (input == '\r' || input == '\n') {
        return;
    }

    parseCommand(std::string(1, static_cast<char>(input)));
}

void TestInterface::setDriver(TMC2209Driver& driver) {
    mDriver = &driver;
}

bool TestInterface::hasCommand() {
    return !commandQueue.empty();
}

CmdType TestInterface::getCommand() {
    if (!commandQueue.empty()) {
        CmdType cmd = commandQueue.front();
        commandQueue.pop();
        return cmd;
    }
    return CmdType::NONE;
}

void TestInterface::sendResponse(CmdType cmd, ResponseType response, const std::string& reason) {
    printf("%s cmd=%s reason=%s\n",
           toString(response),
           toString(cmd),
           reason.c_str());
}

void TestInterface::sendEvent(CmdType cmd, EventType type, EventReason reason) {
    printf("EVENT cmd=%s type=%s reason=%s\n",
           toString(cmd),
           toString(type),
           toString(reason));
}

void TestInterface::logf(const char* format, ...) {
    if (!sStallDebugEnabled || format == nullptr) {
        return;
    }

    char buffer[160];

    va_list args;
    va_start(args, format);
    const int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length <= 0) {
        return;
    }

    printf("%s", buffer);
}

void TestInterface::parseCommand(const std::string& line) {
    if (line == "p") {
        printf("CMD PING\n");
        commandQueue.push(CmdType::PING);
    }
    else if (line == "o") {
        printf("CMD OPEN\n");
        commandQueue.push(CmdType::OPEN);
    }
    else if (line == "c") {
        printf("CMD CLOSE\n");
        commandQueue.push(CmdType::CLOSE);
    }
    else if (line == "s") {
        printf("CMD STOP\n");
        commandQueue.push(CmdType::STOP);
    }
    else if (line == "t") {
        if (mDriver == nullptr) {
            printf("DRIVER UART TEST UNAVAILABLE\n");
        } else {
            static constexpr uint32_t TestStepDelayUs = 1000;
            const uint32_t uartEnableGconf =
                TMC2209Bits::GCONF::I_SCALE_ANALOG |
                TMC2209Bits::GCONF::PDN_DISABLE |
                TMC2209Bits::GCONF::MULTISTEP_FILT;

            uint8_t writeCounterBefore = 0;
            uint8_t writeCounterAfter = 0;
            uint32_t gconf = 0;

            printf("DRIVER UART TEST START\n");

            if (!mDriver->getWriteCounter(writeCounterBefore)) {
                printf("  IFCNT READ FAILED, TRYING GCONF WAKEUP\n");
                mDriver->writeRegister(TMC2209Reg::GCONF, uartEnableGconf);
                sleep_us(TestStepDelayUs);

                if (!mDriver->getWriteCounter(writeCounterBefore)) {
                    printf("  IFCNT READ FAILED AFTER WAKEUP\n");
                    printf("DRIVER UART TEST ERROR\n");
                    return;
                }
            }

            printf("  IFCNT BEFORE=%u\n", static_cast<unsigned>(writeCounterBefore));
            sleep_us(TestStepDelayUs);

            if (!mDriver->readRegister(TMC2209Reg::GCONF, gconf)) {
                printf("  GCONF READ FAILED\n");
                printf("DRIVER UART TEST ERROR\n");
                return;
            }

            printf("  GCONF=0x%08lx\n", static_cast<unsigned long>(gconf));
            sleep_us(TestStepDelayUs);

            mDriver->writeRegister(TMC2209Reg::GCONF, gconf);
            sleep_us(TestStepDelayUs);

            if (!mDriver->getWriteCounter(writeCounterAfter)) {
                printf("  IFCNT READ FAILED AFTER WRITEBACK\n");
                printf("DRIVER UART TEST ERROR\n");
                return;
            }

            printf("  IFCNT AFTER=%u\n", static_cast<unsigned>(writeCounterAfter));

            if (static_cast<uint8_t>(writeCounterBefore + 1u) != writeCounterAfter) {
                printf("  IFCNT MISMATCH EXPECTED=%u ACTUAL=%u\n",
                       static_cast<unsigned>(static_cast<uint8_t>(writeCounterBefore + 1u)),
                       static_cast<unsigned>(writeCounterAfter));
                printf("DRIVER UART TEST ERROR\n");
                return;
            }

            printf("DRIVER UART TEST OK\n");
        }
    }
    else if (line == "h") {
        printf("CMDS:\n");
        printf("  p = PING\n");
        printf("  o = OPEN\n");
        printf("  c = CLOSE\n");
        printf("  s = STOP\n");
        printf("  t = DRIVER UART TEST\n");
        printf("  d = STALL DEBUG TOGGLE\n");
        printf("  h = HELP\n");
    }
    else if (line == "d") {
        sStallDebugEnabled = !sStallDebugEnabled;
        printf("STALL DEBUG %s\n", sStallDebugEnabled ? "ON" : "OFF");
    }
    else {
        printf("CMD NONE\n");
        commandQueue.push(CmdType::NONE);
    }
}
