#pragma once
#include "Gripper.h"
#include "interface/Interface.h"
#include "interface/TestInterface.h"


class Controller {
public:
    Controller(TestInterface& testInterface, Gripper& gripper) : mInterface(testInterface), mGripper(gripper) {};
    
	void update();

private:
    TestInterface& mInterface;
    Gripper& mGripper;

    void openCommand();
    void closeCommand();
    void sendMoveCompletionEvent(const GripperMoveEvent& moveEvent);

};
