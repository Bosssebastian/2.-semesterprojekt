#pragma once
#include <atomic>
#include <thread>

class IoRunner {
public:
    IoRunner();
    ~IoRunner();

    void start();
    void stop();

private:
    void run();

    std::thread mWorker;
    std::atomic<bool> mRunning{false};
};
