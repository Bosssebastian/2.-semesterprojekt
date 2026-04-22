#pragma once
#include <string>
#include <vector>

class SerialPort {
public:
    bool hasLine();
    std::string getLine();
    std::vector<std::string> split(const std::string& line) const;
    void sendLine(const std::string& line);

private:
    std::string mRxBuffer;
    bool mLineReady = false;
};
