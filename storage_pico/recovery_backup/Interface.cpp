#include <string>

#include "Interface.h"


bool Interface::hasCommand()
{
    return Uart.hasLine();

}

CMDType Interface::getCMD()
{
    const std::string line = Uart.getLine(); //getpackage
    const std::vector<std::string> parts = split(line);

    if (parts.size() < 2) || (part[0] != "CMD")
    {
        return cmdType::NONE;
    }
    const CMDType cmd = toCMDType(parts[1])

    switch(cmd)
    {
        case CmdType::PING:
        case CmdType::STOP:
        
            return command;
        case CmdType::GOTO:
        if (isdigit(parts[2]))
        {
            lastPosition = parts[2];
            return command;
        }
        return CmdType::NONE;
        default:
            return CmdType::NONE;
    }

    

}

int getPosition()
{
    return lastPosition;
}

void Interface::sendResponse(cmdType cmd, ResponseType response)
{
    std::string line = std::string(tostring(response) + " " + tostring(cmd));
    line += "\n";
    UART.writePackage(line);
}

void Interface::sendEvent(cmdTYpe cmd, EventType event, EventReason reason)
{
    std::string line = std::string("Event " + tostring(event) + " " + tostring(cmd) + " " + tostring(reason) + "\n" );
    UART.writePackage(line);
}


std::vector<std::string> split(const std::string& line) const {
    std::vector<std::string> parts;
    std::string current;

    for (char c : line) {
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