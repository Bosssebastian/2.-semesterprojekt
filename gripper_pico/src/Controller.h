#pragma once
#include "Gripper.h"
#include "interface/Interface.h"


class Controller {
public:
    Controller(Interface& interface, Gripper& gripper) : mInterface(interface), mGripper(gripper) {};
    
	void update();

private:
    Interface& mInterface;
    Gripper& mGripper;

    void openCommand();
    void closeCommand();
    void sendMoveCompletionEvent(const GripperMoveEvent& moveEvent);

};
