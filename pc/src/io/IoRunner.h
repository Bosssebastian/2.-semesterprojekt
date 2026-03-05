#pragma once
#include <string>
#include <vector>
using namespace std;

class IoRunner {
public:
    IoRunner();
    ~IoRunner();

    void start();
    void stop();

private:

    void run();
};
