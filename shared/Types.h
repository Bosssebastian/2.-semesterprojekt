#pragma once

#include <cstdint>
#include <string>

enum class CmdType {
    NONE,
    PING,
    OPEN,
    CLOSE,
    STOP,
    STATUS,
    STATISTICS,
    GOTO,
    RESET,
    OPEN_RESET,
    OPEN_RESET_FORWARD,
    CURRENT_EVENTS_ON,
    CURRENT_EVENTS_OFF,
    STALL_VALUES_ON,
    STALL_VALUES_OFF
};
enum class ResponseType {OK, ERROR};
enum class EventType {UNKNOWN, ERROR, MOVE_DONE, OPEN_SEQUENCE_DONE, CURRENT_POSITION};
enum class EventReason {NONE, STEPS_FINISHED, STALL, STOPPED, MOVE_ERROR, REACHED_POSITION};

inline const char* toString(CmdType type) {
    switch (type) {
        case CmdType::NONE:
            return "NONE";
        case CmdType::PING:
            return "PING";
        case CmdType::OPEN:
            return "OPEN";
        case CmdType::CLOSE:
            return "CLOSE";
        case CmdType::STOP:
            return "STOP";
        case CmdType::STATUS:
            return "STATUS";
        case CmdType::STATISTICS:
            return "STATISTICS";
        case CmdType::GOTO:
            return "GOTO";
        case CmdType::RESET:
            return "RESET";
        case CmdType::OPEN_RESET:
            return "OPEN_RESET";
        case CmdType::OPEN_RESET_FORWARD:
            return "OPEN_RESET_FORWARD";
        case CmdType::CURRENT_EVENTS_ON:
            return "CURRENT_EVENTS_ON";
        case CmdType::CURRENT_EVENTS_OFF:
            return "CURRENT_EVENTS_OFF";
        case CmdType::STALL_VALUES_ON:
            return "STALL_VALUES_ON";
        case CmdType::STALL_VALUES_OFF:
            return "STALL_VALUES_OFF";
    }

    return "UNKNOWN";
}

inline const char* toString(ResponseType type) {
    switch (type) {
        case ResponseType::OK:
            return "OK";
        case ResponseType::ERROR:
            return "ERROR";
    }

    return "UNKNOWN";
}

inline const char* toString(EventType type) {
    switch (type) {
        case EventType::UNKNOWN:
            return "UNKNOWN";
        case EventType::ERROR:
            return "ERROR";
        case EventType::MOVE_DONE:
            return "MOVE_DONE";
        case EventType::OPEN_SEQUENCE_DONE:
            return "OPEN_SEQUENCE_DONE";
        case EventType::CURRENT_POSITION:
            return "CURRENT_POSITION";
    }

    return "UNKNOWN";
}

inline const char* toString(EventReason reason) {
    switch (reason) {
        case EventReason::NONE:
            return "NONE";
        case EventReason::STEPS_FINISHED:
            return "STEPS_FINISHED";
        case EventReason::STALL:
            return "STALL";
        case EventReason::STOPPED:
            return "STOPPED";
        case EventReason::MOVE_ERROR:
            return "MOVE_ERROR";
        case EventReason::REACHED_POSITION:
            return "REACHED_POSITION";
    }

    return "UNKNOWN";
}

inline CmdType toCmdType(const std::string& value) {
    if (value == "NONE") {
        return CmdType::NONE;
    }
    if (value == "PING") {
        return CmdType::PING;
    }
    if (value == "OPEN") {
        return CmdType::OPEN;
    }
    if (value == "CLOSE") {
        return CmdType::CLOSE;
    }
    if (value == "STOP") {
        return CmdType::STOP;
    }
    if (value == "STATUS") {
        return CmdType::STATUS;
    }
    if (value == "STATISTICS") {
        return CmdType::STATISTICS;
    }
    if (value == "GOTO") {
        return CmdType::GOTO;
    }
    if (value == "RESET") {
        return CmdType::RESET;
    }
    if (value == "OPEN_RESET") {
        return CmdType::OPEN_RESET;
    }
    if (value == "OPEN_RESET_FORWARD") {
        return CmdType::OPEN_RESET_FORWARD;
    }
    if (value == "CURRENT_EVENTS_ON") {
        return CmdType::CURRENT_EVENTS_ON;
    }
    if (value == "CURRENT_EVENTS_OFF") {
        return CmdType::CURRENT_EVENTS_OFF;
    }
    if (value == "STALL_VALUES_ON") {
        return CmdType::STALL_VALUES_ON;
    }
    if (value == "STALL_VALUES_OFF") {
        return CmdType::STALL_VALUES_OFF;
    }

    return CmdType::NONE;
}

inline ResponseType toResponseType(const std::string& value) {
    if (value == "OK") {
        return ResponseType::OK;
    }
    if (value == "ERROR") {
        return ResponseType::ERROR;
    }

    return ResponseType::ERROR;
}

inline EventType toEventType(const std::string& value) {
    if (value == "ERROR") {
        return EventType::ERROR;
    }
    if (value == "MOVE_DONE") {
        return EventType::MOVE_DONE;
    }
    if (value == "OPEN_SEQUENCE_DONE") {
        return EventType::OPEN_SEQUENCE_DONE;
    }
    if (value == "CURRENT_POSITION") {
        return EventType::CURRENT_POSITION;
    }

    return EventType::UNKNOWN;
}

inline EventReason toEventReason(const std::string& value) {
    if (value == "NONE") {
        return EventReason::NONE;
    }
    if (value == "STEPS_FINISHED") {
        return EventReason::STEPS_FINISHED;
    }
    if (value == "STALL") {
        return EventReason::STALL;
    }
    if (value == "STOPPED") {
        return EventReason::STOPPED;
    }
    if (value == "MOVE_ERROR") {
        return EventReason::MOVE_ERROR;
    }
    if (value == "REACHED_POSITION") return EventReason::REACHED_POSITION;

    return EventReason::NONE;
}
