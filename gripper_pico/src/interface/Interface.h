# pragma once
#include "Uart.h"


enum class CmdType{PING, OPEN, CLOSE, STOP, STATUS, STATISTICS};   
enum class EventType{ERROR, GRIPPER};  
enum class ResponseType{OK, ERR};
enum class Event{NONE};
  
class Interface {  
public:  
	void setup();            // Setup of Uart class  
	void update();           // Loop update function  
  
	bool hasCommand();       // Checks if a new command was received  
	CmdType getCommand();    // Gets what type the received command is
  
	void sendResponse(CmdType, ResponseType);   // Sends OK / ERR back
	void sendEvent(EventType, Event);           // Sends event

private:
	Uart uart;

};
