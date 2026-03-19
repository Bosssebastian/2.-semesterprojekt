#include <stdio.h>

#include "Controller.h"
#include "Gripper.h"
#include "interface/Interface.h"

#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    Gripper gripper;
    gripper.setup();

    Interface interface;
    interface.setup();

    Controller controller(gripper, interface);
    controller.setup();

    while (true) {
        gripper.update();
        interface.update();
        controller.update();

        tight_loop_contents();
    }
}
