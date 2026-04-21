#include "IoRunner.h"
#include "SerialPort.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#ifndef _WIN32
#include <dirent.h>
#endif
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {
constexpr int kBaudRate = 115200;

std::vector<std::string> discoverCandidatePorts() {
    std::vector<std::string> ports;

#ifdef _WIN32
    for (int i = 1; i <= 16; ++i) {
        ports.push_back("\\\\.\\COM" + std::to_string(i));
    }

    return ports;
#else
    DIR* dir = opendir("/dev");
    if (dir == nullptr) {
        std::printf("Failed to scan /dev for serial devices\n");
        return ports;
    }

    while (dirent* entry = readdir(dir)) {
        const std::string name = entry->d_name;
        if (name.rfind("ttyACM", 0) == 0 || name.rfind("ttyUSB", 0) == 0) {
            ports.push_back("/dev/" + name);
        }
    }

    closedir(dir);
    std::sort(ports.begin(), ports.end());
    return ports;
#endif
}
}

IoRunner::IoRunner() = default;

IoRunner::~IoRunner() {
    stop();
}

void IoRunner::start() {
    if (mRunning.exchange(true)) {
        return;
    }

    const std::vector<std::string> ports = discoverCandidatePorts();
    if (ports.empty()) {
#ifdef _WIN32
        std::printf("No candidate Windows serial ports were generated\n");
        std::printf("Set up serial port discovery or configure the COM port paths directly\n");
#else
        std::printf("No serial devices found under /dev/ttyACM* or /dev/ttyUSB*\n");
        std::printf("Check that the Pico is connected and that this user can access serial devices\n");
#endif
    } else {
        std::printf("Found %zu candidate serial port(s)\n", ports.size());
    }

    for (const std::string& portPath : ports) {
        std::printf("Probing %s...\n", portPath.c_str());
        SerialPort port(portPath, kBaudRate);
        port.setup();

        if (!port.lastProbeResponse().empty()) {
            std::printf("Probe reply from %s: %s\n", portPath.c_str(), port.lastProbeResponse().c_str());
        } else {
            std::printf("Probe reply from %s: <none>\n", portPath.c_str());
        }

        if (port.identifiedDevice() == "GRIPPER") {
            mGripper.setDevicePath(port.devicePath());
            std::printf("Assigned %s as GRIPPER\n", portPath.c_str());
        } else if (port.identifiedDevice() == "STORAGE") {
            mStorage.setDevicePath(port.devicePath());
            std::printf("Assigned %s as STORAGE\n", portPath.c_str());
        } else {
            std::printf("Ignoring %s because it did not identify as GRIPPER or STORAGE\n", portPath.c_str());
        }
    }

    mGripper.setup();
    mStorage.setup();
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
        mGripper.update();
        mStorage.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

Interface& IoRunner::gripper() {
    return mGripper;
}

Interface& IoRunner::storage() {
    return mStorage;
}
