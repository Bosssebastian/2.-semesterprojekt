#include "Interface.h"
#include "uartClass.h"
#include "../../../shared/Types.h"
#include <string>
#include <vector>

Interface::Interface()
    : uart(0, 0, 1, 115200) {
}

void Interface::setup() {
    uart.setup();
}

bool Interface::hasCommand() {
    return uart.hasPackage();
}

CmdType Interface::getCommand() {
    std::vector<std::string> parts;
    std::string current = "";

    std::string package = uart.readPackage();

    // split package into parts
    for (char c : package) {
        if (c == ' ')
        {
            if (current != "")
            {
                parts.push_back(current);
                current = "";
            }
        }
        else
        {
            current += c;
        }
    }

    // add last char 
    if (current != "")
    {
        parts.push_back(current);
    }

    if (parts.size() < 2 || parts[0] != "CMD") {
        return CmdType::NONE;
    }

    const std::string& command = parts[1];

    if (command == "PING") {
        return CmdType::PING;
    }
    if (command == "OPEN") {
        return CmdType::OPEN;
    }
    if (command == "CLOSE") {
        return CmdType::CLOSE;
    }
    if (command == "STOP") {
        return CmdType::STOP;
    }
    if (command == "STATUS") {
        return CmdType::STATUS;
    }
    if (command == "STATISTICS") {
        return CmdType::STATISTICS;
    }

    return CmdType::NONE;
}

void Interface::sendResponse(CmdType cmd, ResponseType response) {
    uart.writePackage(std::string(toString(response)) + " " + toString(cmd));
}

void Interface::sendStatus() {
    //placeholder for now
}

void Interface::sendStatistics() {
    //placeholder for now
}

void Interface::sendEvent(CmdType cmd, EventType type, EventReason reason) {
    uart.writePackage(std::string("EVENT ") + toString(type) + " " + toString(cmd) + " " + toString(reason));
}
