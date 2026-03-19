#pragma once

#include <cstdint>

enum class CmdType {NONE, PING, OPEN, CLOSE, STOP, STATUS, STATISTICS};
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
