#include "IoRunner.h"
#include <thread>
#include <atomic>
using namespace std;

class IO {
public:
    void start() {
        running = true;
        worker = thread(&IO::run, this);
    }

    void stop() {
        running = false;
        if (worker.joinable())
            worker.join();
    }

private:
    void run() {
        while (running) {
            // io loop
        }
    }

    thread worker;
    atomic<bool> running{false};
};