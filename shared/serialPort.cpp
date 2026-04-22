#include "serialPort.h"
#include <cstdio>
#include "pico/stdlib.h"


bool SerialPort::hasLine() {
    if (mLineReady) {
        return true;
    }

    while (true) {
        const int input = getchar_timeout_us(0);
        if (input == PICO_ERROR_TIMEOUT) {
            return false;
        }

        const char c = static_cast<char>(input);
        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            mLineReady = true;
            return true;
        }

        mRxBuffer += c;
    }
}

std::string SerialPort::getLine() {
    std::string line = mRxBuffer;
    mRxBuffer.clear();
    mLineReady = false;
    return line;
}

std::vector<std::string> SerialPort::split(const std::string& line) const {
    std::vector<std::string> parts;
    std::string current;

    for (char c : line) {
        if (c == ' ') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        parts.push_back(current);
    }

    return parts;
}

void SerialPort::sendLine(const std::string& line) {
    std::printf("%s", line.c_str());
}
