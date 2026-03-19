#pragma once

enum class CmdType {NONE, PING, OPEN, CLOSE, STOP, STATUS, STATISTICS};
enum class ResponseType {OK, ERROR};
enum class EventType {ERROR, Move_DONE};
