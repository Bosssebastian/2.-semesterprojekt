#include "IoRunner.h"
#include <chrono>
#include <thread>

IoRunner::IoRunner() = default;

IoRunner::~IoRunner() {
    stop();
}

void IoRunner::start() {
    if (mRunning.exchange(true)) {
        return;
    }

    mWorker = std::thread(&IoRunner::run, this);
}

void IoRunner::stop() {
    mRunning = false;
    if (mWorker.joinable()) {
        mWorker.join();
    }
}

void IoRunner::run() {
    while (mRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
