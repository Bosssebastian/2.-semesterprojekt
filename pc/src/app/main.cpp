#include <iostream>
#include <limits>
#include "io/IoRunner.h"
#include "vision/VisionRunner.h"
#include "orchestrator/Orchestrator.h"

int main() {
    IoRunner io;
    VisionRunner vision;
    Orchestrator orchestrator(io, vision);

    io.start();
    vision.start();
    orchestrator.run();

    std::cout << "VisionRunner started. Press Enter to stop..." << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    io.stop();
    vision.stop();
    return 0;
}
