#pragma once
#include <atomic>
#include <thread>

class VisionRunner {
public:
    VisionRunner();
    ~VisionRunner();

    void start();
    void stop();
    
private:
    void run();

    cv::VideoCapture cam;
    std::thread mWorker;
    std::atomic<bool> mRunning{false};
};
