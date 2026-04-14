#pragma once
#include "uartClass.h"
#include "../../../shared/Types.h"
#include <cstdint>
#include <string>
  
class Interface {  
public:  
	Interface();
	void setup();
  
	bool hasCommand();       // Checks if a new command was received  
	CmdType getCommand();    // Gets what type the received command is
  
	void sendResponse(CmdType, ResponseType);   // Sends OK / ERR back
    void sendStatus();			//Maybe easier to have its own function?
    void sendStatistics();		//Maybe easier to have its own function?
	void sendEvent(CmdType, EventType, EventReason reason = EventReason::NONE);

private:
	UartClass uart;
};
