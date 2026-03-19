#pragma once
#include <string>


class Uart {  
public:  
	void setup();                  // Setup of pins
	void update();                 // Loop update function
	void sendLine(std::string);    // Sends one message

	bool hasLine();                // Checks if we have recived a full message
	std::string getLine();         // Gets the full message that was recived

private:

};
