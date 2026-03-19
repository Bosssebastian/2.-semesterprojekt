#include "Interface.h"
#include "Uart.h"

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

void Interface::sendResponse(CmdType, ResponseType) {
}

void Interface::sendEvent(EventType, Event) {
}
