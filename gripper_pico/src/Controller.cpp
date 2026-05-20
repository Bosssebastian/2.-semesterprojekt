#include "Controller.h"
#include "Gripper.h"
#include "interface/Interface.h"
#include "../../shared/Types.h"

void Controller::update() {  
    if (mInterface.hasCommand()) {
        CmdType cmd = mInterface.getCommand();
        switch (cmd) {
            case CmdType::PING:
                mInterface.sendResponse(CmdType::PING, ResponseType::OK, "GRIPPER");
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
            case CmdType::RESET:
                resetCommand();
                break;
            case CmdType::STATUS:
                statusCommand();
                break;
            case CmdType::STATISTICS:
                // ToDo
                break;
            case CmdType::CURRENT_EVENTS_ON:
                setCurrentEvents(true);
                break;
            case CmdType::CURRENT_EVENTS_OFF:
                setCurrentEvents(false);
                break;
            case CmdType::STALL_VALUES_ON:
                setStallValues(true);
                break;
            case CmdType::STALL_VALUES_OFF:
                setStallValues(false);
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

void Controller::resetCommand() {
    if (mGripper.reset()) {
        mInterface.sendResponse(CmdType::RESET, ResponseType::OK);
    } else {
        mInterface.sendResponse(CmdType::RESET, ResponseType::ERROR, "BUSY");
    }
}

void Controller::statusCommand() {
    mInterface.sendResponse(CmdType::STATUS, ResponseType::OK, mGripper.isBusy() ? "BUSY" : "IDLE");
}

void Controller::setCurrentEvents(bool enabled) {
    mCurrentSensor.setEventsEnabled(enabled);
    mInterface.sendResponse(enabled ? CmdType::CURRENT_EVENTS_ON : CmdType::CURRENT_EVENTS_OFF, ResponseType::OK);
}

void Controller::setStallValues(bool enabled) {
    mGripper.axis().setStallValueEventsEnabled(enabled);
    mInterface.sendResponse(enabled ? CmdType::STALL_VALUES_ON : CmdType::STALL_VALUES_OFF, ResponseType::OK);
}

void Controller::sendMoveCompletionEvent(const GripperMoveEvent& moveEvent) {
    const EventType doneEvent = moveEvent.eventType;

    switch (moveEvent.result) {
        case GripperMoveResult::Done:
            mInterface.sendEvent(moveEvent.cmd, doneEvent, EventReason::STEPS_FINISHED);
            break;
        case GripperMoveResult::Stalled:
            mInterface.sendEvent(moveEvent.cmd, doneEvent, EventReason::STALL);
            break;
        case GripperMoveResult::Stopped:
            mInterface.sendEvent(moveEvent.cmd, doneEvent, EventReason::STOPPED);
            break;
        case GripperMoveResult::Error:
            mInterface.sendEvent(moveEvent.cmd, EventType::ERROR, EventReason::MOVE_ERROR);
            break;
        case GripperMoveResult::None:
            break;
    }
}
