#include "IoRunner.h"
#include "SerialPort.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#ifndef _WIN32
#include <dirent.h>
#endif
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include "logging/Logger.h"

namespace {
constexpr int kBaudRate = 115200;
constexpr char kPicoVendorId[] = "2e8a";

/*
bool isRaspberryPiPicoPort(const std::string& portPath) {
#ifdef _WIN32
    (void)portPath;
    return true;
#else
    namespace fs = std::filesystem;

    const fs::path ttyName = fs::path(portPath).filename();
    const fs::path sysfsDevicePath = fs::path("/sys/class/tty") / ttyName / "device";

    std::error_code error;
    fs::path devicePath = fs::canonical(sysfsDevicePath, error);
    if (error) {
        return true;
    }

    while (!devicePath.empty() && devicePath != devicePath.parent_path()) {
        const fs::path vendorFile = devicePath / "idVendor";
        std::ifstream vendorStream(vendorFile);
        if (vendorStream.is_open()) {
            std::string vendorId;
            std::getline(vendorStream, vendorId);
            return vendorId == kPicoVendorId;
        }

        devicePath = devicePath.parent_path();
    }

    return true;
#endif
}
*/

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
        LOG_ERROR("Failed to scan for serial devices");
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
        LOG_WARN("No candidate Windows serial ports were generated.");
#else
        LOG_WARN("No serial devices were found.");
#endif
    } else {
        LOG_INFO(("Found " + std::to_string(ports.size()) + " serial port(s)").c_str());
    }
    /*
    for (const std::string& portPath : ports) {
        if (!isRaspberryPiPicoPort(portPath)) {
            LOG_WARN(("Ignoring " + portPath + " because it is not a Raspberry Pi Pico serial device").c_str());
            continue;
        }

        SerialPort port(portPath, kBaudRate);
        port.setup();

        if (port.identifiedDevice() == "GRIPPER") {
            mGripper.setDevicePath(port.devicePath());
            LOG_INFO(("Assigned " + portPath + " as GRIPPER").c_str());
        } else if (port.identifiedDevice() == "STORAGE") {
            mStorage.setDevicePath(port.devicePath());
            LOG_INFO(("Assigned " + portPath + " as STORAGE").c_str());

        } else {
            LOG_WARN(("Ignoring " + portPath + " because it did not identify as GRIPPER or STORAGE").c_str());
        }
    }

    mGripper.setup();
    mStorage.setup();
    mWorker = std::thread(&IoRunner::run, this);*/
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
