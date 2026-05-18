#include <iostream>
#include <limits>
#include "io/IoRunner.h"
#include "vision/VisionRunner.h"
#include "orchestrator/Orchestrator.h"
#include "web/WebServerRunner.h"

int main() {
    IoRunner io;
    VisionRunner vision;
    WebServerRunner web(io.gripper());
    Orchestrator orchestrator(io, vision, web);

    io.start();
    vision.start();
    web.start();
    orchestrator.run();

    io.stop();
    vision.stop();
    web.stop();
    return 0;
}
