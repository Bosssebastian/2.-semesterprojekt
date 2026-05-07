#pragma once
#include <cstdint>
#include <queue>
#include <string>
#include "../../../shared/Types.h"
#include "pico/stdlib.h"

class TMC2209Driver;

class TestInterface {  
public:  
	void setup();
	void update(); 
    void setDriver(TMC2209Driver& driver);
  
	bool hasCommand();       // Checks if a new command was received  
	CmdType getCommand();    // Gets what type the received command is
  
	void sendResponse(CmdType, ResponseType, const std::string& reason = "");   // Sends OK / ERR back
    //void sendStatus();			//Maybe easier to have its own function?
    //void sendStatistics();		//Maybe easier to have its own function?
	void sendEvent(CmdType, EventType, EventReason reason = EventReason::NONE);
    static void logf(const char* format, ...);

private:
    static constexpr uint32_t UsbStartupDelayMs = 2000;
    static bool sStallDebugEnabled;

    TMC2209Driver* mDriver = nullptr;
    std::queue<CmdType> commandQueue;

    void parseCommand(const std::string& line);
};
