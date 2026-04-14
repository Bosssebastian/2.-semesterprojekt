#include "Controller.h"
#include "Gripper.h"
#include "interface/Interface.h"
#include "interface/TestInterface.h"
#include "../../shared/Types.h"

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

    if (mGripper.hasMoveEvent()) {
        sendMoveCompletionEvent(mGripper.getMoveEvent());
    }
}

void Controller::openCommand() {
    if (mGripper.open(true)) {
        mInterface.sendResponse(CmdType::OPEN, ResponseType::OK);
    } else {
        mInterface.sendResponse(CmdType::OPEN, ResponseType::ERROR, "BUSY");
    }
}

void Controller::closeCommand() {
    if (mGripper.close(true)) {
        mInterface.sendResponse(CmdType::CLOSE, ResponseType::OK);
    } else {
        mInterface.sendResponse(CmdType::CLOSE, ResponseType::ERROR, "BUSY");
    }
}

void Controller::sendMoveCompletionEvent(const GripperMoveEvent& moveEvent) {
    switch (moveEvent.result) {
        case GripperMoveResult::Done:
            mInterface.sendEvent(moveEvent.cmd, EventType::MOVE_DONE, EventReason::STEPS_FINISHED);
            break;
        case GripperMoveResult::Stalled:
            mInterface.sendEvent(moveEvent.cmd, EventType::MOVE_DONE, EventReason::STALL);
            break;
        case GripperMoveResult::Stopped:
            mInterface.sendEvent(moveEvent.cmd, EventType::MOVE_DONE, EventReason::STOPPED);
            break;
        case GripperMoveResult::Error:
            mInterface.sendEvent(moveEvent.cmd, EventType::ERROR, EventReason::MOVE_ERROR);
            break;
        case GripperMoveResult::None:
            break;
    }
}
