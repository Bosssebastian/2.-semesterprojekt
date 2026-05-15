#include <iostream>
#include <limits>
//#include "io/IoRunner.h"
#include "vision/VisionRunner.h"
//#include "orchestrator/Orchestrator.h"
//#include "web/WebServerRunner.h"

int main() {
    //IoRunner io;
    VisionRunner vision;
    //WebServerRunner web(io.gripper());
    //Orchestrator orchestrator(io, vision, web);

    //io.start();
    vision.start();
    //web.start();
    //orchestrator.run();

    std::cout << "VisionRunner started. Press Enter to test..." << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    vision.scanForObject();

    std::cout << "VisionRunner started. Press Enter to stop..." << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    //io.stop();
    vision.stop();
    //web.stop();
    return 0;
}
