#pragma once
#include <atomic>
#include <thread>
#include "VisionController.h"

class VisionRunner {
public:
    VisionRunner();
    ~VisionRunner();

    void start();
    void stop();

    void test();
    
private:
    void run();

    VisionController mVision;
    std::thread mWorker;
    std::atomic<bool> mRunning{false};
};
