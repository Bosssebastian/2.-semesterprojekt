#pragma once
#include "CurrentSensor.h"
#include "Gripper.h"
#include "interface/Interface.h"


class Controller {
public:
    Controller(Interface& interface, Gripper& gripper, CurrentSensor& currentSensor)
        : mInterface(interface), mGripper(gripper), mCurrentSensor(currentSensor) {};
    
	void update();

private:
    Interface& mInterface;
    Gripper& mGripper;
    CurrentSensor& mCurrentSensor;

    void openCommand();
    void closeCommand();
    void resetCommand();
    void statusCommand();
    void setCurrentEvents(bool enabled);
    void setStallValues(bool enabled);
    void sendMoveCompletionEvent(const GripperMoveEvent& moveEvent);

};
