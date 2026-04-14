#pragma once
#include "Types.h"
#include "UartClass.h"
#include <ctime>
#include <string>
#include <map>
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
    Interface(int uartId, int pinTx = 0, int pinRx = 1, int baud = 115200);

    void setup();
    void update();

    bool sendCommand(CmdType command, const std::string& argument = "");
    CmdStatus getStatus(CmdType cmd) const;

private:
    UartClass mUart;
    std::map<CmdType, CmdState> mCmdStates;

    void handlePackage(const std::vector<std::string>& parts);
    void handleAcknowledgment(const std::vector<std::string>& parts);
    void handleEvent(const std::vector<std::string>& parts);
    void handleTimeouts();
    static std::vector<std::string> split(const std::string& str);
};
