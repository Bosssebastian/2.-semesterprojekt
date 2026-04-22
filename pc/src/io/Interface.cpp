#include "Interface.h"
#include <ctime>
#include <utility>
#include <vector>

namespace {
constexpr double kCommandTimeoutSeconds = 5.0;

bool commandWaitsForEvent(CmdType command) {
    return command == CmdType::OPEN || command == CmdType::CLOSE || command == CmdType::GOTO;
}
}

Interface::Interface(std::string devicePath, int baud)
    : mSerialPort(std::move(devicePath), baud) {
}

void Interface::setDevicePath(std::string devicePath) {
    mSerialPort.setDevicePath(std::move(devicePath));
}

void Interface::setup() {
    mSerialPort.setup();
}

void Interface::update() {
    while (mSerialPort.hasPackage()) {
        std::string message = mSerialPort.readPackage();
        handlePackage(split(message));
    }

    handleTimeouts();
}

bool Interface::sendCommand(CmdType command, const std::string& argument) {
    CmdState& state = mCmdStates[command];

    if (state.status == CmdStatus::WAITING_FOR_ACK || state.status == CmdStatus::WAITING_FOR_RESULT) {
        return false;
    }

    state.active = true;
    state.status = CmdStatus::WAITING_FOR_ACK;
    state.retryCount = 0;
    state.timestamp = std::time(nullptr);

    std::string package = std::string("CMD ") + toString(command);
    if (!argument.empty()) {
        package += " " + argument;
    }
    package += "\n";

    mSerialPort.writePackage(package);
    return true;
}

CmdStatus Interface::getStatus(CmdType cmd) const {
    const auto it = mCmdStates.find(cmd);
    if (it == mCmdStates.end()) {
        return CmdStatus::IDLE;
    }

    return it->second.status;
}

std::vector<std::string> Interface::split(const std::string& str) {
    std::vector<std::string> parts;
    std::string current;

    for (char c : str) {
        if (c == ' ') {
            if (!current.empty()) {
                parts.push_back(std::move(current));
                current = "";
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        parts.push_back(std::move(current));
    }

    return parts;
}

void Interface::handlePackage(const std::vector<std::string>& parts) {
    if (parts.empty()) {
        return;
    }

    if (parts[0] == "DEBUG") {
        return;
    }

    if (parts[0] == "OK" || parts[0] == "ERROR") {
        handleAcknowledgment(parts);
        return;
    }

    if (parts[0] == "EVENT") {
        handleEvent(parts);
    }
}

void Interface::handleAcknowledgment(const std::vector<std::string>& parts) {
    if (parts.size() < 2) {
        return;
    }

    CmdType cmd = toCmdType(parts[1]);
    CmdState& state = mCmdStates[cmd];

    if (parts[0] == "OK") {
        if (commandWaitsForEvent(cmd)) {
            state.status = CmdStatus::WAITING_FOR_RESULT;
            state.timestamp = std::time(nullptr);
        } else {
            state.status = CmdStatus::DONE;
            state.active = false;
        }
    } else {
        state.status = CmdStatus::FAILED;
        state.active = false;
    }
}

void Interface::handleEvent(const std::vector<std::string>& parts) {
    if (parts.size() < 4) {
        return;
    }

    EventType eventType = toEventType(parts[1]);
    CmdType cmd = toCmdType(parts[2]);
    CmdState& state = mCmdStates[cmd];

    if (eventType == EventType::MOVE_DONE) {
        state.status = CmdStatus::DONE;
    } else {
        state.status = CmdStatus::FAILED;
    }

    state.active = false;
}

void Interface::handleTimeouts() {
    const std::time_t currentTime = std::time(nullptr);

    for (auto& pair : mCmdStates) {
        CmdState& state = pair.second;

        if (!state.active) {
            continue;
        }

        if (std::difftime(currentTime, state.timestamp) > kCommandTimeoutSeconds) {
            state.status = CmdStatus::TIMED_OUT;
            state.active = false;
        }
    }
}
