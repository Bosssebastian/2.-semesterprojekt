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

    io.stop();
    vision.stop();
    return 0;
}
