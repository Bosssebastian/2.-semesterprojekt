#include "VisionRunner.h"
#include <chrono>
#include <thread>

using namespace std;

VisionRunner::VisionRunner() {
    mVision.scanForObject();
}

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
    while (mRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}