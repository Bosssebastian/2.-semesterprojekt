#include <stdio.h>

#include "Controller.h"
#include "Gripper.h"
#include "interface/Interface.h"
#include "interface/TestInterface.h"

#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    Gripper gripper;
    gripper.setup();

    Interface interface;
    interface.setup();

    TestInterface testInterface;
    testInterface.setup();
    testInterface.setDriver(gripper.driver());

    Controller controller(interface, gripper);
    controller.setup();

    while (true) {
        gripper.update();
        testInterface.update();
        controller.update();

        tight_loop_contents();
    }
}
