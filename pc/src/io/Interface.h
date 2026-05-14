#pragma once
#include "Types.h"
#include "SerialPort.h"
#include <ctime>
#include <map>
#include <mutex>
#include <string>
#include <vector>

enum class CmdStatus {
    IDLE,
    WAITING_FOR_ACK,
    WAITING_FOR_RESULT,
    DONE,
    FAILED,
    TIMED_OUT
};

struct CmdState {
    CmdStatus status = CmdStatus::IDLE;
    int retryCount = 0;
    bool active = false;
    time_t timestamp = 0;
};

struct CurrentSample {
    uint32_t timestampMs = 0;
    float amps = 0.0f;
};

class Interface {
public:
    Interface(std::string devicePath, std::string portLabel = "", int baud = 115200);

    void setDevicePath(std::string devicePath);
    void setup();
    void update();

    bool sendCommand(CmdType command, const std::string& argument = "");
    CmdStatus getStatus(CmdType cmd) const;
    std::vector<CurrentSample> getRecentCurrentSamples(uint32_t windowMs) const;

private:
    static constexpr std::size_t CurrentSampleCapacity = 30000;

    SerialPort mSerialPort;
    std::map<CmdType, CmdState> mCmdStates;
    std::vector<CurrentSample> mCurrentSamples;
    std::size_t mCurrentWriteIndex = 0;
    bool mCurrentBufferWrapped = false;
    mutable std::mutex mCurrentMutex;

    void handlePackage(const std::vector<std::string>& parts);
    void handleAcknowledgment(const std::vector<std::string>& parts);
    void handleEvent(const std::vector<std::string>& parts);
    void handleCurrentEvent(const std::vector<std::string>& parts);
    void storeCurrentSample(uint32_t timestampMs, float amps);
    void handleTimeouts();
    static std::vector<std::string> split(const std::string& str);
};
