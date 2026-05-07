#pragma once
#include <string>
#include <vector>

#include "uartClass.h"
#include "../../shared/Types.h"

class Interface
{
public:
    Interface(UartClass& uart);

    bool hasCommand();
    CmdType getCommand();

    int getPosition() const;

    void sendResponse(CmdType cmd, ResponseType response, const std::string& message);
    void sendEvent(CmdType cmd, EventType event, EventReason reason);
    void sendCurrentPosition(int position);

private:
    UartClass& uart;
    int lastPosition;

    std::vector<std::string> split(const std::string& line) const;
    bool isNumber(const std::string& text) const;

};