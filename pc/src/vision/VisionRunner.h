#pragma once
#include <atomic>
#include <thread>
#include "VisionController.h"
#include <vector>

class VisionRunner {
public:
    VisionRunner();
    ~VisionRunner();

    void start();
    void stop();
    void getPos(std::vector<std::vector<double>>& ,double&);
    void getInfo(std::string& object, std::string& size, std::string& color);
    bool objectReady();
    void scanForObject();
    void stopScan();
    
private:
    void run();
    VisionController mVision;   
    std::thread mWorker;
    std::atomic<bool> mRunning{false};
};
