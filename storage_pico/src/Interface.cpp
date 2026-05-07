#include "Interface.h"

#include <string>
#include <vector>
#include <cctype>

Interface::Interface(UartClass& uart)
    :uart(uart)
{

}

bool Interface::hasCommand()
{
    return uart.hasPackage();
}

CmdType Interface::getCommand()
{
    const std::string line = uart.readPackage(); //getpackage
    const std::vector<std::string> parts = split(line);

    if (parts.size() < 2 || parts[0] != "CMD")
    {
        return CmdType::NONE;
    }
    const CmdType cmd = toCmdType(parts[1]);

    switch(cmd)
    {
        case CmdType::PING:
        case CmdType::STOP:
        case CmdType::STATUS:
        case CmdType::RESET:
            return cmd;

        case CmdType::GOTO:
            if (parts.size() >= 3 && isNumber(parts[2]))
            {
                lastPosition = std::stoi(parts[2]);
                return cmd;
            }
            return CmdType::NONE;

        default:
            return CmdType::NONE;
    }

    

}

int Interface::getPosition() const
{
    return lastPosition;
}

void Interface::sendResponse(CmdType cmd, ResponseType response, const std::string& message)
{
    std::string line = std::string(toString(response)) + " " + toString(cmd);
    line += "\n";
    uart.writePackage(line);
}

void Interface::sendEvent(CmdType cmd, EventType event, EventReason reason)
{
    std::string line = std::string("Event ") + toString(event) + " " + toString(cmd) + " " + toString(reason) + "\n";
    uart.writePackage(line);
}

void Interface::sendCurrentPosition(int position)
{
    std::string line = std::string("EVENT ") + toString(EventType::CURRENT_POSITION) + " " + std::to_string(position) + "\n";
    uart.writePackage(line);
}


std::vector<std::string> Interface::split(const std::string& line) const {
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

bool Interface::isNumber(const std::string& text) const
{
    if (text.empty())
    {
        return false;
    }

    for (char c : text)
    {
        if (!std::isdigit(static_cast<unsigned char>(c)))
        {
            return false;
        }
    }
    return true;
}