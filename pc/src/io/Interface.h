#pragma once
#include "Types.h"
#include "SerialPort.h"
#include "Uart.h"
#include <ctime>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "Uart.h"

enum class CmdStatus {
    IDLE,
    WAITING_FOR_ACK,
    WAITING_FOR_RESULT,
    DONE,
    FAILED,
    TIMED_OUT
};

inline const char* toString(CmdStatus status) {
    switch (status) {
        case CmdStatus::IDLE:
            return "IDLE";
        case CmdStatus::WAITING_FOR_ACK:
            return "WAITING_FOR_ACK";
        case CmdStatus::WAITING_FOR_RESULT:
            return "WAITING_FOR_RESULT";
        case CmdStatus::DONE:
            return "DONE";
        case CmdStatus::FAILED:
            return "FAILED";
        case CmdStatus::TIMED_OUT:
            return "TIMED_OUT";
    }

    return "UNKNOWN";
}

enum class configType {
    SERIALPORT,
    UART
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
    Interface(std::string devicePath, std::string portLabel = "", double commandTimeoutSeconds = 20.0, int baud = 115200);
    Interface(std::string devicePath, configType configuration, std::string portLabel = "", double commandTimeoutSeconds = 20.0, int baud = 115200);

    void setDevicePath(std::string devicePath);
    void setup();
    void update();

    bool sendCommand(CmdType command, const std::string& argument = "");
    CmdStatus getStatus(CmdType cmd) const;
    bool isDeviceBusy() const;
    void resetCommandStates();
    std::vector<CurrentSample> getRecentCurrentSamples(uint32_t windowMs) const;

private:
    static constexpr std::size_t CurrentSampleCapacity = 30000;

    configType mConfiguration = configType::SERIALPORT;
    SerialPort mSerialPort;
    UartClass mUart;
    double mCommandTimeoutSeconds;
    std::map<CmdType, CmdState> mCmdStates;
    std::vector<CurrentSample> mCurrentSamples;
    std::size_t mCurrentWriteIndex = 0;
    bool mCurrentBufferWrapped = false;
    bool mDeviceBusy = false;
    mutable std::mutex mCurrentMutex;

    void handlePackage(const std::vector<std::string>& parts);
    void handleAcknowledgment(const std::vector<std::string>& parts);
    void handleEvent(const std::vector<std::string>& parts);
    void handleCurrentEvent(const std::vector<std::string>& parts);
    void storeCurrentSample(uint32_t timestampMs, float amps);
    void handleTimeouts();
    static std::vector<std::string> split(const std::string& str);
};
