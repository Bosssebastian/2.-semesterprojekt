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

void VisionRunner::test()
{
    for (int i = 0; i < 1000; i++)
    {
        mVision.scanForObject();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        if(!(i % 50))
        {
            std::cout << i << "\n";
        }
    }
    std::cout << "Spotted: " << mVision.spot_test << "/1000\n" << "Read: " << mVision.read_test << "/1000\n";
    mVision.spot_test = 0;
    mVision.read_test = 0;
}

void VisionRunner::run() {
    while (mRunning) {
        mVision.scanForObject();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}