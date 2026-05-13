#include "Interface.h"
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
constexpr double kCommandTimeoutSeconds = 20.0;

bool commandWaitsForEvent(CmdType command) {
    return command == CmdType::OPEN || command == CmdType::CLOSE || command == CmdType::GOTO || command == CmdType::RESET;
}
}

Interface::Interface(std::string devicePath, configType mConfiguration, std::string portLabel, int baud) {
    
    if (mConfiguration == configType::SERIALPORT) {
        mSerialPort(std::move(devicePath), baud, std::move(portLabel))
    } else {
        mUart() //Remember to input parameters
    }

    mCurrentSamples.resize(CurrentSampleCapacity);
}

void Interface::setDevicePath(std::string devicePath) {
    if (mConfiguration == configType::SERIALPORT)
    {
        mSerialPort.setDevicePath(std::move(devicePath));
    }
    
}

void Interface::setup() {
    if (mConfiguration == configType::SERIALPORT) {
        mSerialPort.setup();
    } else {
        mUart.setup();
    }
}

void Interface::update() {

    if (mConfiguration == configType::SERIALPORT) {
        while (mSerialPort.hasPackage()) {
            std::string message = mSerialPort.readPackage();
            handlePackage(split(message));
        }
    } else {
        while (mUart.hasLine()) {
            std::string message = mUart.getLine();
            handlePackage(split(message));
        }
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
    if (mConfiguration == configType::SERIALPORT) {
        mSerialPort.writePackage(package);
    } else {
       mUart.sendLine(package); 
    }
   
    return true;
}

CmdStatus Interface::getStatus(CmdType cmd) const {
    const auto it = mCmdStates.find(cmd);
    if (it == mCmdStates.end()) {
        return CmdStatus::IDLE;
    }

    return it->second.status;
}

std::vector<CurrentSample> Interface::getRecentCurrentSamples(uint32_t windowMs) const {
    std::lock_guard<std::mutex> lock(mCurrentMutex);

    const std::size_t sampleCount = mCurrentBufferWrapped ? mCurrentSamples.size() : mCurrentWriteIndex;
    if (sampleCount == 0) {
        return {};
    }

    std::vector<CurrentSample> ordered;
    ordered.reserve(sampleCount);

    if (mCurrentBufferWrapped) {
        ordered.insert(ordered.end(), mCurrentSamples.begin() + static_cast<std::ptrdiff_t>(mCurrentWriteIndex), mCurrentSamples.end());
        ordered.insert(ordered.end(), mCurrentSamples.begin(), mCurrentSamples.begin() + static_cast<std::ptrdiff_t>(mCurrentWriteIndex));
    } else {
        ordered.insert(ordered.end(), mCurrentSamples.begin(), mCurrentSamples.begin() + static_cast<std::ptrdiff_t>(mCurrentWriteIndex));
    }

    if (windowMs == 0 || ordered.empty()) {
        return ordered;
    }

    const uint32_t newestTimestamp = ordered.back().timestampMs;
    const uint32_t oldestTimestamp = (newestTimestamp > windowMs) ? newestTimestamp - windowMs : 0;
    const auto firstRecent = std::lower_bound(
        ordered.begin(),
        ordered.end(),
        oldestTimestamp,
        [](const CurrentSample& sample, uint32_t timestamp) {
            return sample.timestampMs < timestamp;
        });

    return std::vector<CurrentSample>(firstRecent, ordered.end());
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

    if (parts[0] == "EVENT" && parts.size() >= 5 && parts[1] == "CURRENT") {
        handleCurrentEvent(parts);
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

void Interface::handleCurrentEvent(const std::vector<std::string>& parts) {
    try {
        const uint32_t startMs = static_cast<uint32_t>(std::stoul(parts[2]));
        const uint32_t periodMs = static_cast<uint32_t>(std::stoul(parts[3]));

        for (std::size_t sampleIndex = 4; sampleIndex < parts.size(); ++sampleIndex) {
            const float amps = std::stof(parts[sampleIndex]);
            const uint32_t timestampMs = startMs + static_cast<uint32_t>(sampleIndex - 4) * periodMs;
            storeCurrentSample(timestampMs, amps);
        }
    } catch (const std::exception&) {
        return;
    }
}

void Interface::storeCurrentSample(uint32_t timestampMs, float amps) {
    std::lock_guard<std::mutex> lock(mCurrentMutex);

    mCurrentSamples[mCurrentWriteIndex] = CurrentSample{timestampMs, amps};
    mCurrentWriteIndex = (mCurrentWriteIndex + 1) % mCurrentSamples.size();
    if (mCurrentWriteIndex == 0) {
        mCurrentBufferWrapped = true;
    }
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
