#include "Interface.h"
#include "PinConfig.h"
#include "uartClass.h"
#include <string>
#include <vector>

Interface::Interface()
    : uart(PinConfig::UART_PORT == uart1 ? 1 : 0,
           PinConfig::UART_TX_PIN,
           PinConfig::UART_RX_PIN,
           PinConfig::UART_BAUD) {
}

void Interface::setup() {
    uart.setup();
}

bool Interface::hasCommand() {
    return uart.hasPackage();
}

CmdType Interface::getCommand() {
    const std::vector<std::string> parts = split(uart.readPackage());

    if (parts.size() < 2 || parts[0] != "CMD") {
        return CmdType::NONE;
    }

    const CmdType command = toCmdType(parts[1]);
    switch (command) {
        case CmdType::PING:
        case CmdType::OPEN:
        case CmdType::CLOSE:
        case CmdType::STOP:
        case CmdType::STATUS:
        case CmdType::STATISTICS:
            return command;
        default:
            return CmdType::NONE;
    }
}

void Interface::sendResponse(CmdType cmd, ResponseType response, const std::string& reason) {
    std::string line = std::string(toString(response)) + " " + toString(cmd);
    if (!reason.empty()) {
        line += " " + reason;
    }
    line += "\n";
    uart.writePackage(line);
}

void Interface::sendStatus() {
    //placeholder for now
}

void Interface::sendStatistics() {
    //placeholder for now
}

void Interface::sendEvent(CmdType cmd, EventType type, EventReason reason) {
    uart.writePackage(std::string("EVENT ") + toString(type) + " " + toString(cmd) + " " + toString(reason) + "\n");
}

std::vector<std::string> Interface::split(const std::string& package) {
    std::vector<std::string> parts;
    std::string current;

    for (char c : package) {
        if (c == ' ') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        parts.push_back(current);
    }

    return parts;
}
