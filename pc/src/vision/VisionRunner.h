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
    void getPos(std::vector<std::vector<double>>);
    bool objectReady();
    void run();
    
private:
    VisionController mVision;
    std::thread mWorker;
    std::atomic<bool> mRunning{false};
};
