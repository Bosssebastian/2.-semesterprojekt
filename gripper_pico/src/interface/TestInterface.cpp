#include "TestInterface.h"
#include "Types.h"
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
    else if (line == "h") {
        printf("CMDS:\n");
        printf("  p = PING\n");
        printf("  o = OPEN\n");
        printf("  c = CLOSE\n");
        printf("  s = STOP\n");
        printf("  t = STALL DEBUG TOGGLE\n");
        printf("  h = HELP\n");
    }
    else if (line == "t") {
        sStallDebugEnabled = !sStallDebugEnabled;
        printf("STALL DEBUG %s\n", sStallDebugEnabled ? "ON" : "OFF");
    }
    else {
        printf("CMD NONE\n");
        commandQueue.push(CmdType::NONE);
    }
}
