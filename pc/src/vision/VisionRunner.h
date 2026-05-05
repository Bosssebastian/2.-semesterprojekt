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

    VisionController mVision;
    
private:
    void run();

    std::thread mWorker;
    std::atomic<bool> mRunning{false};
};
