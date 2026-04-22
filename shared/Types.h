#pragma once

#include <cstdint>
#include <string>

enum class CmdType {NONE, PING, OPEN, CLOSE, STOP, STATUS, STATISTICS, GOTO};
enum class ResponseType {OK, ERROR};
enum class EventType {ERROR, MOVE_DONE};
enum class EventReason {NONE, STEPS_FINISHED, STALL, STOPPED, MOVE_ERROR};

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
        case EventType::ERROR:
            return "ERROR";
        case EventType::MOVE_DONE:
            return "MOVE_DONE";
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

    return EventType::ERROR;
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

    return EventReason::NONE;
}
