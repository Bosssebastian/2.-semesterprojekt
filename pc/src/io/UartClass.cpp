#include "UartClass.h"

#include <cstdio>

#ifdef _WIN32

UartClass::UartClass(int uart_id, int pin_tx, int pin_rx, int baud)
    : mUartId(uart_id), mPinTx(pin_tx), mPinRx(pin_rx), mBaud(baud) {
}

void UartClass::setup() {
    std::printf("PC UartClass is not implemented for Windows\n");
}

void UartClass::writePackage(std::string line) {
    (void)line;
}

std::string UartClass::readPackage() {
    mPackageReady = false;
    return "";
}

bool UartClass::hasPackage() {
    return false;
}

#else

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace {
const char* devicePathFromId(int uartId) {
    switch (uartId) {
        case 0:
            return "/dev/ttyTHS0";
        case 1:
            return "/dev/ttyTHS1";
        default:
            return "/dev/ttyTHS0";
    }
}

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
}

UartClass::UartClass(int uart_id, int pin_tx, int pin_rx, int baud)
    : mUartId(uart_id), mPinTx(pin_tx), mPinRx(pin_rx), mBaud(baud) {
}

void UartClass::setup() {
    (void)mPinTx;
    (void)mPinRx;

    const char* devicePath = devicePathFromId(mUartId);
    mFd = open(devicePath, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (mFd < 0) {
        std::printf("Failed to open UART device %s: %s\n", devicePath, std::strerror(errno));
        return;
    }

    termios tty{};
    if (tcgetattr(mFd, &tty) != 0) {
        std::printf("Failed to read UART attributes for %s: %s\n", devicePath, std::strerror(errno));
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
        std::printf("Failed to configure UART device %s: %s\n", devicePath, std::strerror(errno));
        close(mFd);
        mFd = -1;
    }
}

void UartClass::writePackage(std::string line) {
    if (mFd < 0) {
        return;
    }

    const ssize_t bytesWritten = write(mFd, line.c_str(), line.size());
    if (bytesWritten < 0) {
        std::printf("Failed to write UART package: %s\n", std::strerror(errno));
    }
}

std::string UartClass::readPackage() {
    while (!hasPackage()) {
    }

    std::string line = mRxBuffer;
    mRxBuffer.clear();
    mPackageReady = false;
    return line;
}

bool UartClass::hasPackage() {
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

#endif
