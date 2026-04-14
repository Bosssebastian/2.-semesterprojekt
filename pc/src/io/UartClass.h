#pragma once

#include <string>

class UartClass {
public:
    UartClass(int uart_id, int pin_tx, int pin_rx, int baud);

    void setup();
    void writePackage(std::string line);
    std::string readPackage();
    bool hasPackage();

private:
    int mUartId;
    int mPinTx;
    int mPinRx;
    int mBaud;
    std::string mRxBuffer;
    bool mPackageReady{false};
#ifdef _WIN32
    void* mHandle{nullptr};
#else
    int mFd{-1};
#endif
};
