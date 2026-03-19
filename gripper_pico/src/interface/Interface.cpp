#include "Interface.h"

#include <string>

void Interface::setup() {
    uart.setup();
}

void Interface::update() {
    uart.update();
    // loop code
}

bool Interface::hasCommand() {
    return false;
}

CmdType Interface::getCommand() {
    return CmdType::PING;
}

void Interface::sendResponse(CmdType cmd, ResponseType response, const std::string& reason) {

}

void Interface::sendStatus() {

}

void Interface::sendStatistics() {

}

void Interface::sendEvent(EventType, Event) {
}
