#include "Controller.h"
#include "Gripper.h"
#include "interface/Interface.h"
#include "interface/TestInterface.h"
#include "Types.h"

void Controller::setup() {  
}

void Controller::update() {  
    if (mInterface.hasCommand()) {
        CmdType cmd = mInterface.getCommand();
        switch (cmd) {
            case CmdType::PING:
                mInterface.sendResponse(CmdType::PING, ResponseType::OK);
                break;
            case CmdType::OPEN:
                openCommand();
                break;
            case CmdType::CLOSE:
                closeCommand();
                break;
            case CmdType::STOP:
                mGripper.stop();
                mInterface.sendResponse(CmdType::STOP, ResponseType::OK);
                break;
            case CmdType::STATUS:
                // ToDo
                break;
            case CmdType::STATISTICS:
                // ToDo
                break;
            default:
                mInterface.sendResponse(cmd, ResponseType::ERROR);
                break;
        }
    }
}

void Controller::openCommand() {
    if (mGripper.open()) {
        mInterface.sendResponse(CmdType::OPEN, ResponseType::OK);
    } else {
        mInterface.sendResponse(CmdType::OPEN, ResponseType::ERROR, "BUSY");
    }
}

void Controller::closeCommand() {
    if (mGripper.close()) {
        mInterface.sendResponse(CmdType::CLOSE, ResponseType::OK);
    } else {
        mInterface.sendResponse(CmdType::CLOSE, ResponseType::ERROR, "BUSY");
    }
}