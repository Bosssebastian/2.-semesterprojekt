#include "Uart.h"
#include "logging/Logger.h"
#include <cerrno>
#include <iostream> 
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <sstream>

namespace {
constexpr const char* SerialDevicePath = "/dev/ttyAMA0"; //Changed From "/dev/serial0"

std::string errnoMessage(const std::string& action, int errorNumber)
{
    std::ostringstream message;
    message << action << ": errno=" << errorNumber << " (" << std::strerror(errorNumber) << ")";
    return message.str();
}

std::string printableLine(std::string line)
{
    if (!line.empty() && line.back() == '\n')
    {
        line.pop_back();
    }

    return line;
}
}


UartClass::UartClass(int pin_tx, int pin_rx, int baud)
{
    this->uart_id = 0;
    this->pin_tx = pin_tx;
    this->pin_rx = pin_rx;
    this->baud = baud;
}

void UartClass::setup()
{
    LOG_INFO("UART setup starting");
    LOG_INFO(std::string("UART opening device: ") + SerialDevicePath);

    serial_fd = open(SerialDevicePath, O_RDWR | O_NOCTTY | O_SYNC);

    if (serial_fd < 0)
    {
        const int errorNumber = errno;
        LOG_ERROR(errnoMessage(std::string("UART open failed for ") + SerialDevicePath, errorNumber));
        return;
    }

    LOG_INFO(std::string("UART open succeeded for ") + SerialDevicePath);

    termios tty{};

    if (tcgetattr(serial_fd, &tty) != 0)
    {
        const int errorNumber = errno;
        LOG_ERROR(errnoMessage("UART tcgetattr failed", errorNumber));
        close(serial_fd);
        serial_fd = -1;
        return;
    }

    // Baud rate
    speed_t speed;

    switch (baud)
    {
        case 9600:
            speed = B9600;
            break;

        case 19200:
            speed = B19200;
            break;

        case 38400:
            speed = B38400;
            break;

        case 57600:
            speed = B57600;
            break;

        case 115200:
            speed = B115200;
            break;

        default:
            LOG_ERROR("Unsupported baud rate: " + std::to_string(baud));
            close(serial_fd);
            serial_fd = -1;
            return;
    }

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    // 8 data bits
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    // 1 stop bit
    tty.c_cflag &= ~CSTOPB;

    // No parity
    tty.c_cflag &= ~PARENB;

    // Enable receiver and ignore modem-control lines
    tty.c_cflag |= CREAD | CLOCAL;

    // Raw input mode
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // Disable software flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Disable special handling of received bytes
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    // Raw output mode
    tty.c_oflag &= ~OPOST;

    // Read settings
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0)
    {
        const int errorNumber = errno;
        LOG_ERROR(errnoMessage("UART tcsetattr failed", errorNumber));
        close(serial_fd);
        serial_fd = -1;
        return;
    }

    LOG_INFO("UART settings applied successfully");
    LOG_INFO("UART opened on " + std::string(SerialDevicePath) + " at " + std::to_string(baud) + " baud");
}

void UartClass::sendLine(std::string line)
{
    if (serial_fd < 0)
    {
        LOG_ERROR("UART send skipped: serial_fd is not open");
        return;
    }

    LOG_INFO("UART TX: " + printableLine(line));

    const ssize_t bytesWritten = write(serial_fd, line.c_str(), line.length());
    const ssize_t bytesRequested = static_cast<ssize_t>(line.length());

    std::ostringstream result;
    result << "UART write result: requested " << bytesRequested << " bytes, wrote " << bytesWritten << " bytes";

    if (bytesWritten < 0)
    {
        const int errorNumber = errno;
        LOG_ERROR(result.str() + "; " + errnoMessage("write failed", errorNumber));
        return;
    }

    if (bytesWritten != bytesRequested)
    {
        const int errorNumber = errno;
        LOG_ERROR(result.str() + "; " + errnoMessage("short write", errorNumber));
        return;
    }

    LOG_INFO(result.str());
}

std::string UartClass::getLine()
{
    while (!hasLine()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::string line = rxBuffer;
    rxBuffer.clear();
    mLineReady = false;
    return line;
}

bool UartClass::hasLine()
{
    if (mLineReady) {
        return true;
    }

    if (serial_fd < 0)
    {
        return false;
    }

    char c;

    while (true)
    {
        ssize_t bytesRead = read(serial_fd, &c, 1);

        if (bytesRead > 0)
        {
            if (c == '\r')
            {
                continue;
            }

            if (c == '\n')
            {
                mLineReady = true;
                return true;
            }

            rxBuffer += c;
        }
        else if (bytesRead == 0)
        {
            // No more data available right now
            return false;
        }
        else
        {
            // read() returned -1
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return false;
            }

            return false;
        }
    }
}

std::vector<std::string> UartClass::split(const std::string& line) const
{
    std::vector<std::string> parts;
    std::string current;

    for (char c : line)
    {
        if (c == ' ')
        {
            if (!current.empty())
            {
                parts.push_back(current);
                current.clear();
            }
        }
        else 
        {
            current += c;
        }
    }
    if (!current.empty())
    {
        parts.push_back(current);
    }

    return parts;
}

/*
void UartClass::clearRXQue() {
    printf("Clearing RX Que\n");
    while (uart_is_readable(uart)) {
        uart_getc(uart);
    }

}
*/
