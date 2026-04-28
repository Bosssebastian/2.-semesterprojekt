#pragma once

#include <string>

class SerialPort {
public:
    SerialPort(std::string devicePath, int baud, std::string portLabel = "");
    ~SerialPort();

    void setDevicePath(std::string devicePath);
    void setup();
    void writePackage(std::string line);
    std::string readPackage();
    bool hasPackage();
    bool tryReadPackage(std::string& line, int timeoutMs);
    const std::string& identifiedDevice() const;
    const std::string& lastProbeResponse() const;
    const std::string& devicePath() const;

private:
    void closePort();

    std::string mDevicePath;
    std::string mIdentifiedDevice;
    std::string mLastProbeResponse;
    std::string mPortLabel;
    int mBaud;
    std::string mRxBuffer;
    bool mPackageReady{false};
#ifdef _WIN32
    void* mHandle{nullptr};
#else
    int mFd{-1};
#endif
};
