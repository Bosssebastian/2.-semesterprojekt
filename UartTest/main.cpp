#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <chrono>
#include <thread>

class PiUart
{
public:
    PiUart(const std::string& device, int baudrate)
        : mDevice(device), mBaudrate(baudrate), mFd(-1)
    {
    }

    bool openPort()
    {
        mFd = open(mDevice.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

        if (mFd < 0)
        {
            std::cerr << "ERROR: Could not open UART device: " << mDevice << std::endl;
            std::cerr << "Reason: " << strerror(errno) << std::endl;
            return false;
        }

        termios tty{};

        if (tcgetattr(mFd, &tty) != 0)
        {
            std::cerr << "ERROR: tcgetattr failed: " << strerror(errno) << std::endl;
            close(mFd);
            mFd = -1;
            return false;
        }

        cfmakeraw(&tty);

        speed_t speed = getBaudConstant(mBaudrate);

        cfsetispeed(&tty, speed);
        cfsetospeed(&tty, speed);

        // 8 data bits
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;

        // No parity
        tty.c_cflag &= ~PARENB;

        // 1 stop bit
        tty.c_cflag &= ~CSTOPB;

        // Disable flow control
        tty.c_cflag &= ~CRTSCTS;

        // Enable receiver, ignore modem control lines
        tty.c_cflag |= CREAD | CLOCAL;

        // Non-blocking read behavior
        tty.c_cc[VMIN] = 0;
        tty.c_cc[VTIME] = 0;

        if (tcsetattr(mFd, TCSANOW, &tty) != 0)
        {
            std::cerr << "ERROR: tcsetattr failed: " << strerror(errno) << std::endl;
            close(mFd);
            mFd = -1;
            return false;
        }

        tcflush(mFd, TCIOFLUSH);

        std::cout << "UART opened successfully on " << mDevice
                  << " at " << mBaudrate << " baud" << std::endl;

        return true;
    }

    void closePort()
    {
        if (mFd >= 0)
        {
            close(mFd);
            mFd = -1;
        }
    }

    bool sendLine(const std::string& line)
    {
        if (mFd < 0)
        {
            return false;
        }

        std::string fullLine = line + "\n";

        ssize_t bytesWritten = write(mFd, fullLine.c_str(), fullLine.size());

        if (bytesWritten < 0)
        {
            std::cerr << "ERROR: UART write failed: " << strerror(errno) << std::endl;
            return false;
        }

        std::cout << "TX: [" << line << "]" << std::endl;
        return true;
    }

    bool hasLine()
    {
        if (mLineReady)
        {
            return true;
        }

        if (mFd < 0)
        {
            return false;
        }

        char buffer[128];

        ssize_t bytesRead = read(mFd, buffer, sizeof(buffer));

        if (bytesRead < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                std::cerr << "ERROR: UART read failed: " << strerror(errno) << std::endl;
            }

            return false;
        }

        for (ssize_t i = 0; i < bytesRead; i++)
        {
            char c = buffer[i];

            if (c == '\r')
            {
                continue;
            }

            if (c == '\n')
            {
                mLineReady = true;
                return true;
            }

            mRxBuffer += c;
        }

        return false;
    }

    std::string getLine()
    {
        mLineReady = false;

        std::string line = mRxBuffer;
        mRxBuffer.clear();

        return line;
    }

private:
    std::string mDevice;
    int mBaudrate;
    int mFd;

    std::string mRxBuffer;
    bool mLineReady = false;

    speed_t getBaudConstant(int baudrate)
    {
        switch (baudrate)
        {
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
            std::cerr << "WARNING: Unsupported baudrate. Defaulting to 115200." << std::endl;
            return B115200;
        }
    }
};

int main()
{
    PiUart uart("/dev/serial0", 115200);

    if (!uart.openPort())
    {
        return 1;
    }

    std::cout << "Pi UART test started." << std::endl;
    std::cout << "Commands will be sent automatically." << std::endl;
    std::cout << "Every received UART line will be printed." << std::endl;

    auto lastSendTime = std::chrono::steady_clock::now();

    while (true)
    {
        // Send test command every 2 seconds
        auto now = std::chrono::steady_clock::now();

        auto elapsedMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSendTime).count();

        if (elapsedMs >= 2000)
        {
            uart.sendLine("CMD PING");
            lastSendTime = now;
        }

        // Read received lines
        if (uart.hasLine())
        {
            std::string line = uart.getLine();
            std::cout << "RX: [" << line << "]" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    uart.closePort();

    return 0;
}