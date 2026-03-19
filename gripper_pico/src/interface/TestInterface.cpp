#include "TestInterface.h"
#include "Types.h"
#include <string>
#include <queue>
#include "pico/stdlib.h"

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
        printf("Get Command: %d\n", static_cast<int>(cmd));
        return cmd;
    }
    return CmdType::NONE;
}

void TestInterface::sendResponse(CmdType cmd, ResponseType response, const std::string& reason) {
    printf("Cmd: %d, Response: %d, Reason: %s\n", static_cast<int>(cmd), static_cast<int>(response), reason.c_str());
}

void TestInterface::sendEvent(CmdType cmd, EventType type, const std::string& reason) {
    // ToDo
}

void TestInterface::parseCommand(const std::string& line) {
    if (line == "p") {
        printf("PING\n");
        commandQueue.push(CmdType::PING);
    }
    else if (line == "o") {
        printf("OPEN\n");
        commandQueue.push(CmdType::OPEN);
    }
    else if (line == "c") {
        printf("CLOSE\n");
        commandQueue.push(CmdType::CLOSE);
    }
    else if (line == "s") {
        printf("STOP\n");
        commandQueue.push(CmdType::STOP);
    }
    else {
        printf("NONE\n");
        commandQueue.push(CmdType::NONE);
    }
}
