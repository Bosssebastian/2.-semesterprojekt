#include "Interface.h"
#include "Types.h"
#include <string>

void Interface::setup() {
    uart.setup();
}

void Interface::update() {
    uart.update();
    // loop code
}

bool Interface::hasCommand() {
    return false;   //Placeholder
}

CmdType Interface::getCommand() {
    return CmdType::PING; //Placeholder
}

void Interface::sendResponse(CmdType cmd, ResponseType response, const std::string& reason) {

}

void Interface::sendStatus() {

}

void Interface::sendStatistics() {

}

void Interface::sendEvent(CmdType cmd, EventType type, const std::string& reason) {
}
