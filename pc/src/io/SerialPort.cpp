#include "SerialPort.h"
#include "logging/Logger.h"
#include <chrono>
#include <cstdio>
#include <thread>
#include <utility>

namespace {
std::string prefixLogMessage(const std::string& portLabel, std::string message) {
    if (portLabel.empty()) {
        return message;
    }

    return "[" + portLabel + "] " + message;
}
}

#ifdef _WIN32

SerialPort::SerialPort(std::string devicePath, int baud, std::string portLabel)
    : mDevicePath(std::move(devicePath)), mPortLabel(std::move(portLabel)), mBaud(baud) {
}

SerialPort::~SerialPort() {
    closePort();
}

void SerialPort::setDevicePath(std::string devicePath) {
    mDevicePath = std::move(devicePath);
}

void SerialPort::setup() {
    mRxBuffer.clear();
    mPackages.clear();
    mPackageReady = false;
    LOG_WARN(prefixLogMessage(mPortLabel, "PC serial transport is not implemented for Windows").c_str());
}

void SerialPort::writePackage(std::string line) {
    (void)line;
}

std::string SerialPort::readPackage() {
    if (mPackages.empty()) {
        mPackageReady = false;
        return "";
    }

    std::string line = std::move(mPackages.front());
    mPackages.pop_front();
    mPackageReady = !mPackages.empty();
    return line;
}

bool SerialPort::hasPackage() {
    return false;
}

bool SerialPort::tryReadPackage(std::string& line, int timeoutMs) {
    (void)line;
    (void)timeoutMs;
    return false;
}

const std::string& SerialPort::identifiedDevice() const {
    return mIdentifiedDevice;
}

const std::string& SerialPort::lastProbeResponse() const {
    return mLastProbeResponse;
}

const std::string& SerialPort::devicePath() const {
    return mDevicePath;
}

const std::string& SerialPort::lastProbeResponse() const {
    return mLastProbeResponse;
}

void SerialPort::closePort() {
}

#else

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace {
constexpr int kProbeResponseTimeoutMs = 400;

speed_t baudToTermios(int baud) {
    switch (baud) {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        default:
            return B115200;
    }
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}
}

SerialPort::SerialPort(std::string devicePath, int baud, std::string portLabel)
    : mDevicePath(std::move(devicePath)), mPortLabel(std::move(portLabel)), mBaud(baud) {
}

SerialPort::~SerialPort() {
    closePort();
}

void SerialPort::setDevicePath(std::string devicePath) {
    mDevicePath = std::move(devicePath);
}

void SerialPort::setup() {
    closePort();
    mIdentifiedDevice.clear();
    mLastProbeResponse.clear();
    mRxBuffer.clear();
    mPackages.clear();
    mPackageReady = false;

    if (mDevicePath.empty()) {
        LOG_WARN(prefixLogMessage(mPortLabel, "No serial device path configured").c_str());
        return;
    }

    mFd = open(mDevicePath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (mFd < 0) {
        LOG_ERROR(prefixLogMessage(
                      mPortLabel,
                      std::string("Failed to open serial device ") + mDevicePath + ": " + std::strerror(errno))
                      .c_str());
        return;
    }

    termios tty{};
    if (tcgetattr(mFd, &tty) != 0) {
        LOG_ERROR(prefixLogMessage(
                      mPortLabel,
                      std::string("Failed to read serial attributes for ") + mDevicePath + ": " + std::strerror(errno))
                      .c_str());
        close(mFd);
        mFd = -1;
        return;
    }

    cfsetispeed(&tty, baudToTermios(mBaud));
    cfsetospeed(&tty, baudToTermios(mBaud));

    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(ICRNL | INLCR);
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(mFd, TCSANOW, &tty) != 0) {
        LOG_ERROR(prefixLogMessage(
                      mPortLabel,
                      std::string("Failed to configure serial device ") + mDevicePath + ": " + std::strerror(errno))
                      .c_str());
        close(mFd);
        mFd = -1;
        return;
    }

    writePackage("CMD PING\n");

    std::string line;
    while (tryReadPackage(line, kProbeResponseTimeoutMs)) {
        if (startsWith(line, "DEBUG ")) {
            continue;
        }

        mLastProbeResponse = line;
        if (startsWith(line, "OK PING ")) {
            mLastProbeResponse = line;
            mIdentifiedDevice = line.substr(std::strlen("OK PING "));
        }

        break;
    }
}

void SerialPort::writePackage(std::string line) {
    if (mFd < 0) {
        return;
    }

    const ssize_t bytesWritten = write(mFd, line.c_str(), line.size());
    if (bytesWritten < 0) {
        LOG_ERROR(prefixLogMessage(
                      mPortLabel,
                      std::string("Failed to write serial package to ") + mDevicePath + ": " + std::strerror(errno))
                      .c_str());
    }
}

std::string SerialPort::readPackage() {
    while (!hasPackage()) {
    }

    std::string line = std::move(mPackages.front());
    mPackages.pop_front();
    mPackageReady = !mPackages.empty();
    return line;
}

bool SerialPort::hasPackage() {
    if (mPackageReady) {
        return true;
    }

    if (mFd < 0) {
        return false;
    }

    char buffer[64];
    const ssize_t bytesRead = read(mFd, buffer, sizeof(buffer));

    if (bytesRead <= 0) {
        return false;
    }

    for (ssize_t i = 0; i < bytesRead; ++i) {
        const char c = buffer[i];

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            mPackages.push_back(std::move(mRxBuffer));
            mRxBuffer.clear();
            continue;
        }

        mRxBuffer += c;
    }

    mPackageReady = !mPackages.empty();
    return mPackageReady;
}

bool SerialPort::tryReadPackage(std::string& line, int timeoutMs) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (std::chrono::steady_clock::now() < deadline) {
        if (hasPackage()) {
            line = readPackage();
            return true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return false;
}

const std::string& SerialPort::identifiedDevice() const {
    return mIdentifiedDevice;
}

const std::string& SerialPort::lastProbeResponse() const {
    return mLastProbeResponse;
}

const std::string& SerialPort::devicePath() const {
    return mDevicePath;
}

/*const std::string& SerialPort::lastProbeResponse() const {
    return mLastProbeResponse;
}*/

void SerialPort::closePort() {
    if (mFd >= 0) {
        close(mFd);
        mFd = -1;
    }
}

#endif
