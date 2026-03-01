#include "VisionRunner.h"
#include <thread>
#include <atomic>
using namespace std;

class Vision {
public:
    void start() {
        running = true;
        worker = thread(&Vision::run, this);
    }

    void stop() {
        running = false;
        if (worker.joinable())
            worker.join();
    }

private:
    void run() {
        while (running) {
            // vision loop
        }
    }

    thread worker;
    atomic<bool> running{false};
};