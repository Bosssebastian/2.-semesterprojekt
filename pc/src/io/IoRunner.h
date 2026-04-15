#pragma once
#include "Interface.h"
#include <atomic>
#include <string>
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

    Interface mGripper{""};
    Interface mStorage{""};
    std::thread mWorker;
    std::atomic<bool> mRunning{false};
};
