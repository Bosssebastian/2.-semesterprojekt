#include "SerialPort.h"

#include <chrono>
#include <cstdio>
#include <thread>
#include <utility>

#ifdef _WIN32

SerialPort::SerialPort(std::string devicePath, int baud)
    : mDevicePath(std::move(devicePath)), mBaud(baud) {
}

SerialPort::~SerialPort() {
    closePort();
}

void SerialPort::setDevicePath(std::string devicePath) {
    mDevicePath = std::move(devicePath);
}

void SerialPort::setup() {
    std::printf("PC serial transport is not implemented for Windows\n");
}

void SerialPort::writePackage(std::string line) {
    (void)line;
}

std::string SerialPort::readPackage() {
    mPackageReady = false;
    return "";
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

SerialPort::SerialPort(std::string devicePath, int baud)
    : mDevicePath(std::move(devicePath)), mBaud(baud) {
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

    if (mDevicePath.empty()) {
        std::printf("No serial device path configured\n");
        return;
    }

    mFd = open(mDevicePath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (mFd < 0) {
        std::printf("Failed to open serial device %s: %s\n", mDevicePath.c_str(), std::strerror(errno));
        return;
    }

    termios tty{};
    if (tcgetattr(mFd, &tty) != 0) {
        std::printf("Failed to read serial attributes for %s: %s\n", mDevicePath.c_str(), std::strerror(errno));
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
        std::printf("Failed to configure serial device %s: %s\n", mDevicePath.c_str(), std::strerror(errno));
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
        std::printf("Failed to write serial package to %s: %s\n", mDevicePath.c_str(), std::strerror(errno));
    }
}

std::string SerialPort::readPackage() {
    while (!hasPackage()) {
    }

    std::string line = mRxBuffer;
    mRxBuffer.clear();
    mPackageReady = false;
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
            mPackageReady = true;
            return true;
        }

        mRxBuffer += c;
    }

    return false;
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

void SerialPort::closePort() {
    if (mFd >= 0) {
        close(mFd);
        mFd = -1;
    }
}

#endif
