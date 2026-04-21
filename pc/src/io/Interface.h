#pragma once
#include "Types.h"
#include "SerialPort.h"
#include <ctime>
#include <map>
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

class Interface {
public:
    Interface(std::string devicePath, std::string portLabel = "", int baud = 115200);

    void setDevicePath(std::string devicePath);
    void setup();
    void update();

    bool sendCommand(CmdType command, const std::string& argument = "");
    CmdStatus getStatus(CmdType cmd) const;

private:
    SerialPort mSerialPort;
    std::map<CmdType, CmdState> mCmdStates;

    void handlePackage(const std::vector<std::string>& parts);
    void handleAcknowledgment(const std::vector<std::string>& parts);
    void handleEvent(const std::vector<std::string>& parts);
    void handleTimeouts();
    static std::vector<std::string> split(const std::string& str);
};
