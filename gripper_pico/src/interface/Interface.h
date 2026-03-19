#pragma once
#include "Uart.h"
#include <cstdint>
#include <string>

enum class CmdType {PING, OPEN, CLOSE, STOP, STATUS, STATISTICS};
enum class ResponseType {OK, ERROR};
enum class EventType {ERROR, Move_DONE};
  
class Interface {  
public:  
	void setup();
	void update(); 
  
	bool hasCommand();       // Checks if a new command was received  
	CmdType getCommand();    // Gets what type the received command is
  
	void sendResponse(CmdType, ResponseType, const std::string& reason = "");   // Sends OK / ERR back
    void sendStatus();			//Maybe easier to have its own function?
    void sendStatistics();		//Maybe easier to have its own function?
	void sendEvent(CmdType, EventType, const std::string& reason = "");

private:
	Uart uart;
};
