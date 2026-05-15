#include <stdio.h>

#include "Controller.h"
#include "CurrentSensor.h"
#include "Gripper.h"
#include "interface/Interface.h"

#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    CurrentSensor currentSensor;
    currentSensor.setup();

    Gripper gripper(currentSensor);
    gripper.setup();

    Interface interface;

    //TestInterface testInterface;
    //testInterface.setup();
    //testInterface.setDriver(gripper.driver());

    Controller controller(interface, gripper, currentSensor);

    while (true) {
        gripper.update();
        currentSensor.update();
        //testInterface.update();
        controller.update();

        tight_loop_contents();
    }
}
