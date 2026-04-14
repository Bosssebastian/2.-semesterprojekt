#pragma once
#include "uartClass.h"
#include "../../../shared/Types.h"
#include <cstdint>
#include <string>
#include <vector>
  
class Interface {  
public:  
	Interface();
	void setup();
  
	bool hasCommand();       // Checks if a new command was received  
	CmdType getCommand();    // Gets what type the received command is
  
	void sendResponse(CmdType, ResponseType, const std::string& reason = "");   // Sends OK / ERR back
    void sendStatus();			//Maybe easier to have its own function?
    void sendStatistics();		//Maybe easier to have its own function?
	void sendEvent(CmdType, EventType, EventReason reason = EventReason::NONE);

private:
	UartClass uart;

    static std::vector<std::string> split(const std::string& package);
};
