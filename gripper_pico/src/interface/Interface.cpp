#include "Interface.h"

void Interface::setup() {
}

void Interface::update() {
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
