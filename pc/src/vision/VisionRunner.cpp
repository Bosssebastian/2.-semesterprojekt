#include "VisionRunner.h"
#include <chrono>
#include <thread>

using namespace std;

VisionRunner::VisionRunner() = default;

VisionRunner::~VisionRunner() {
    stop();
}

void VisionRunner::start() {
    if (mRunning.exchange(true)) {
        return;
    }
    mWorker = std::thread(&VisionRunner::run, this);
}

void VisionRunner::stop() {
    mRunning = false;
    if (mWorker.joinable()) {
        mWorker.join();
    }
}

void VisionRunner::run() {
    mVision.setSameSpot(0);
    mVision.scanForObject();
}

void VisionRunner::getPos(std::vector<std::vector<double>> outputPos){
    mVision.getObjectPosition(outputPos);
}

bool VisionRunner::objectReady(){
    return mVision.objectReady();
}