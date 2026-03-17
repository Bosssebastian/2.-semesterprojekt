#pragma once
#include <stdint.h>
#include <stddef.h>
#include "hardware/uart.h"

class TMC2209Driver {
public:
    TMC2209Driver(uart_inst_t* uartPort, uint32_t baudrate, uint8_t slaveAddress);

    void begin();

    bool testConnection();
    bool writeRegister(uint8_t reg, uint32_t value);
    bool readRegister(uint8_t reg, uint32_t& value);

    bool setCurrent(uint8_t ihold, uint8_t irun, uint8_t iholddelay);
    bool setMicrosteps(uint16_t microsteps);
    bool enableSpreadCycle(bool enable);
    bool setPwmThreshold(uint32_t threshold);
    bool setCoolThreshold(uint32_t threshold);
    bool setStallGuardThreshold(uint8_t threshold);
    bool configureStallGuard(uint32_t coolThreshold, uint8_t stallThreshold);

    bool getWriteCounter(uint8_t& writeCounter);
    bool readDriverStatus(uint32_t& driverStatus);
    bool readDiagState(bool& active);
    bool readStallGuardResult(uint16_t& result);
    bool isStallGuardTriggered(bool& triggered);

private:
    uart_inst_t* mUartPort;
    uint32_t mBaudrate;
    uint8_t mSlaveAddress;

    static constexpr uint8_t Sync = 0x05;
    static constexpr uint8_t WriteBit = 0x80;

    static uint8_t calcCRC(const uint8_t* data, size_t len);
    static uint8_t microstepsToMres(uint16_t microsteps);

    void flushRx();
    bool readExact(uint8_t* dst, size_t len, uint32_t timeoutUs);
};
