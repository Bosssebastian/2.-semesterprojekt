#pragma once
#include "Gripper.h"
#include "interface/Interface.h"


class Controller {
public:
    Controller(Gripper& gripper, Interface& interface) : mGripper(gripper), mInterface(interface) {};
	void setup(); 
	void update();

private:
    void openCommand();
    void closeCommand();

    Gripper& mGripper;
    Interface& mInterface;

};
