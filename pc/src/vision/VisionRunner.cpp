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
    mVision.scanForObject();
}

void VisionRunner::scanForObject(){
    mVision.setState(false);
}

void VisionRunner::stopScan(){
    mVision.setState(true);
}

void VisionRunner::getPos(std::vector<std::vector<double>>& outputPos, double& rotOut){
    mVision.getObjectPosition(outputPos);
    mVision.getObjectRot(rotOut);
}

bool VisionRunner::objectReady(){
    return mVision.objectReady();
}

void VisionRunner::getInfo(std::string& object, std::string& size, std::string& color){
    mVision.getObjectInfo(object, size, color);
    return;
}