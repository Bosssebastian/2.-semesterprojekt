#pragma once
#include <cstdint>
#include <string>
#include <queue>
#include "Types.h"
#include "pico/stdlib.h"

class TestInterface {  
public:  
	void setup();
	void update(); 
  
	bool hasCommand();       // Checks if a new command was received  
	CmdType getCommand();    // Gets what type the received command is
  
	void sendResponse(CmdType, ResponseType, const std::string& reason = "");   // Sends OK / ERR back
    //void sendStatus();			//Maybe easier to have its own function?
    //void sendStatistics();		//Maybe easier to have its own function?
	void sendEvent(CmdType, EventType, const std::string& reason = "");

private:
    static constexpr uint32_t UsbStartupDelayMs = 2000;

    std::queue<CmdType> commandQueue;

    void parseCommand(const std::string& line);
};
