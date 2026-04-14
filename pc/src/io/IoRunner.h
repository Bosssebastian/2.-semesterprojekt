#pragma once
#include "Interface.h"
#include <atomic>
#include <thread>

class IoRunner {
public:
    IoRunner();
    ~IoRunner();

    void start();
    void stop();

    Interface& gripper();
    Interface& storage();

private:
    void run();

    Interface mGripper{0};
    Interface mStorage{1};
    std::thread mWorker;
    std::atomic<bool> mRunning{false};
};
