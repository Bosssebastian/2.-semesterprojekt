#pragma once
#include "Gripper.h"
#include "interface/Interface.h"


class Controller {
public:
    Controller(Interface& interface, Gripper& gripper) : mInterface(interface), mGripper(gripper) {};
	void setup(); 
	void update();

private:
    Gripper& mGripper;
    Interface& mInterface;

    void openCommand();
    void closeCommand();
    void sendMoveCompletionEvent(const GripperMoveEvent& moveEvent);

};
