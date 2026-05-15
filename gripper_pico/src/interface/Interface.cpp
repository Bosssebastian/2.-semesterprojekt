#include "Interface.h"
#include <string>


bool Interface::hasCommand() {
    return mSerialPort.hasLine();
}

CmdType Interface::getCommand() {
    const std::string line = mSerialPort.getLine();
    const std::vector<std::string> parts = mSerialPort.split(line);

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
        case CmdType::RESET:
        case CmdType::CURRENT_EVENTS_ON:
        case CmdType::CURRENT_EVENTS_OFF:
        case CmdType::STALL_VALUES_ON:
        case CmdType::STALL_VALUES_OFF:
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
    mSerialPort.sendLine(line);
}

void Interface::sendStatus() {
    //placeholder for now
}

void Interface::sendStatistics() {
    //placeholder for now
}

void Interface::sendEvent(CmdType cmd, EventType type, EventReason reason) {
    mSerialPort.sendLine(std::string("EVENT ") + toString(type) + " " + toString(cmd) + " " + toString(reason) + "\n");
}
