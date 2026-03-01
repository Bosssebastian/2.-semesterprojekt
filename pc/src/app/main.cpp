#include <iostream>
#include "io\IoRunner.h"
#include "vision\VisionRunner.h"
#include "orchestrator\Orchestrator.h"

using namespace std;


int main() {
    IoRunner io;
    VisionRunner vision;
    Orchestrator orchestrator;

    io.start();
    vision.start();
    orchestrator.run();

    
}