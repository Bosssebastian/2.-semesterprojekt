#include <stdio.h>

#include "Controller.h"
#include "Gripper.h"
#include "interface/Interface.h"
#include "interface/TestInterface.h"

#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    Gripper gripper;
    const bool driver = gripper.setup();

    Interface interface;
    interface.setup();

    TestInterface testInterface(gripper);
    testInterface.setup(driver);

    while (true) {
        gripper.update();
        interface.update();
        testInterface.update();

        tight_loop_contents();
    }
}
