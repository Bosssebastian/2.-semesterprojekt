#include <stdio.h>

#include "Controller.h"
#include "CurrentSensor.h"
#include "Gripper.h"
#include "interface/Interface.h"

#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    Gripper gripper;
    gripper.setup();

    CurrentSensor currentSensor;
    currentSensor.setup();

    Interface interface;

    //TestInterface testInterface;
    //testInterface.setup();
    //testInterface.setDriver(gripper.driver());

    Controller controller(interface, gripper);

    while (true) {
        gripper.update();
        currentSensor.update();
        //testInterface.update();
        controller.update();

        tight_loop_contents();
    }
}
