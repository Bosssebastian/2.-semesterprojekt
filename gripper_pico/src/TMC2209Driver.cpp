#include "TMC2209Driver.h"
#include "PinConfig.h"
#include "TMC2209Registers.h"

#include "pico/time.h"

TMC2209Driver::TMC2209Driver(uart_inst_t* uartPort, uint32_t baudrate, uint8_t slaveAddress)
    : mUartPort(uartPort),
      mBaudrate(baudrate),
      mSlaveAddress(slaveAddress) {}

void TMC2209Driver::begin() {
    gpio_set_function(PinConfig::TMC_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PinConfig::TMC_UART_RX_PIN, GPIO_FUNC_UART);

    uart_init(mUartPort, mBaudrate);
    uart_set_format(mUartPort, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(mUartPort, false);
    flushRx();
}

bool TMC2209Driver::testConnection() {
    uint8_t writeCounterBefore = 0;
    uint8_t writeCounterAfter = 0;
    uint32_t gconf = 0;

    if (!getWriteCounter(writeCounterBefore)) {
        // TMC2208 single-wire UART often needs a first write to GCONF with
        // PDN_DISABLE set before read access becomes usable.
        const uint32_t uartEnableGconf =
            TMC2209Bits::GCONF::I_SCALE_ANALOG |
            TMC2209Bits::GCONF::PDN_DISABLE |
            TMC2209Bits::GCONF::MULTISTEP_FILT;

        if (!writeRegister(TMC2209Reg::GCONF, uartEnableGconf)) {
            return false;
        }

        if (!getWriteCounter(writeCounterBefore)) {
            return false;
        }
    }

    if (!readRegister(TMC2209Reg::GCONF, gconf)) {
        return false;
    }

    if (!writeRegister(TMC2209Reg::GCONF, gconf)) {
        return false;
    }

    if (!getWriteCounter(writeCounterAfter)) {
        return false;
    }

    return static_cast<uint8_t>(writeCounterBefore + 1u) == writeCounterAfter;
}


bool TMC2209Driver::setCurrent(uint8_t ihold, uint8_t irun, uint8_t iholddelay) {
    uint32_t reg = 0;
    reg |= static_cast<uint32_t>(ihold & 0x1F) << 0;
    reg |= static_cast<uint32_t>(irun & 0x1F) << 8;
    reg |= static_cast<uint32_t>(iholddelay & 0x0F) << 16;
    return writeRegister(TMC2209Reg::IHOLD_IRUN, reg);
}

bool TMC2209Driver::setMicrosteps(uint16_t microsteps) {
    uint32_t gconf = 0;
    if (!readRegister(TMC2209Reg::GCONF, gconf)) {
        return false;
    }

    gconf |= TMC2209Bits::GCONF::MSTEP_REG_SELECT;
    if (!writeRegister(TMC2209Reg::GCONF, gconf)) {
        return false;
    }

    uint32_t chopconf = 0;
    if (!readRegister(TMC2209Reg::CHOPCONF, chopconf)) {
        return false;
    }

    chopconf &= ~(0x0Fu << TMC2209Bits::CHOPCONF::MRES_SHIFT);
    chopconf |= static_cast<uint32_t>(microstepsToMres(microsteps)) << TMC2209Bits::CHOPCONF::MRES_SHIFT;

    return writeRegister(TMC2209Reg::CHOPCONF, chopconf);
}

bool TMC2209Driver::enableSpreadCycle(bool enable) {
    uint32_t gconf = 0;
    if (!readRegister(TMC2209Reg::GCONF, gconf)) {
        return false;
    }

    if (enable) {
        gconf |= TMC2209Bits::GCONF::EN_SPREADCYCLE;
    } else {
        gconf &= ~TMC2209Bits::GCONF::EN_SPREADCYCLE;
    }

    return writeRegister(TMC2209Reg::GCONF, gconf);
}

bool TMC2209Driver::setPwmThreshold(uint32_t threshold) {
    return writeRegister(TMC2209Reg::TPWMTHRS, threshold & 0x000FFFFFu);
}

bool TMC2209Driver::setCoolThreshold(uint32_t threshold) {
    return writeRegister(TMC2209Reg::TCOOLTHRS, threshold & 0x000FFFFFu);
}

bool TMC2209Driver::setStallGuardThreshold(uint8_t threshold) {
    return writeRegister(TMC2209Reg::SGTHRS, threshold);
}

bool TMC2209Driver::configureStallGuard(uint32_t coolThreshold, uint8_t stallThreshold) {
    // stallGuard4 is active while TPWMTHRS <= TSTEP <= TCOOLTHRS.
    return setCoolThreshold(coolThreshold) && setStallGuardThreshold(stallThreshold);
}

bool TMC2209Driver::getWriteCounter(uint8_t& writeCounter) {
    uint32_t reg = 0;
    if (!readRegister(TMC2209Reg::IFCNT, reg)) {
        return false;
    }
    writeCounter = static_cast<uint8_t>(reg & 0xFF);
    return true;
}

bool TMC2209Driver::readDriverStatus(uint32_t& driverStatus) {
    return readRegister(TMC2209Reg::DRV_STATUS, driverStatus);
}

bool TMC2209Driver::readDiagState(bool& active) {
    uint32_t ioin = 0;
    if (!readRegister(TMC2209Reg::IOIN, ioin)) {
        return false;
    }

    active = (ioin & TMC2209Bits::IOIN::DIAG) != 0;
    return true;
}

bool TMC2209Driver::readStallGuardResult(uint16_t& result) {
    uint32_t sgResult = 0;
    if (!readRegister(TMC2209Reg::SG_RESULT, sgResult)) {
        return false;
    }

    result = static_cast<uint16_t>(sgResult & 0x03FFu);
    return true;
}

bool TMC2209Driver::isStallGuardTriggered(bool& triggered) {
    uint32_t threshold = 0;
    uint16_t sgResult = 0;

    if (!readRegister(TMC2209Reg::SGTHRS, threshold)) {
        return false;
    }

    if (!readStallGuardResult(sgResult)) {
        return false;
    }

    triggered = sgResult < static_cast<uint16_t>(threshold & 0xFFu);
    return true;
}

bool TMC2209Driver::writeRegister(uint8_t reg, uint32_t value) {
    uint8_t frame[8];
    frame[0] = Sync;
    frame[1] = mSlaveAddress;
    frame[2] = reg | WriteBit;
    frame[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    frame[4] = static_cast<uint8_t>((value >> 16) & 0xFF);
    frame[5] = static_cast<uint8_t>((value >> 8) & 0xFF);
    frame[6] = static_cast<uint8_t>((value >> 0) & 0xFF);
    frame[7] = calcCRC(frame, 7);

    flushRx();
    uart_write_blocking(mUartPort, frame, sizeof(frame));

    return true;
}

bool TMC2209Driver::readRegister(uint8_t reg, uint32_t& value) {
    // Read request frame
    uint8_t request[4];
    request[0] = Sync;
    request[1] = mSlaveAddress;
    request[2] = reg & 0x7F;
    request[3] = calcCRC(request, 3);

    flushRx();
    uart_write_blocking(mUartPort, request, sizeof(request));

    // On the TMC2208 single-wire UART, RX often sees the 4-byte request echo
    // before the 8-byte read reply. Scan the received stream for a valid reply.
    uint8_t rx[16] = {0};
    size_t rxLen = 0;
    const absolute_time_t deadline = make_timeout_time_us(10000);

    while (absolute_time_diff_us(get_absolute_time(), deadline) > 0 && rxLen < sizeof(rx)) {
        if (!uart_is_readable(mUartPort)) {
            continue;
        }

        rx[rxLen++] = uart_getc(mUartPort);

        if (rxLen < 8) {
            continue;
        }

        for (size_t start = 0; start <= rxLen - 8; ++start) {
            const uint8_t* reply = &rx[start];
            if (reply[0] != Sync || reply[1] != 0xFF || reply[2] != reg) {
                continue;
            }

            const uint8_t crc = calcCRC(reply, 7);
            if (crc != reply[7]) {
                continue;
            }

            value = 0;
            value |= static_cast<uint32_t>(reply[3]) << 24;
            value |= static_cast<uint32_t>(reply[4]) << 16;
            value |= static_cast<uint32_t>(reply[5]) << 8;
            value |= static_cast<uint32_t>(reply[6]) << 0;
            return true;
        }
    }

    return false;
}

uint8_t TMC2209Driver::calcCRC(const uint8_t* data, size_t len) {
    uint8_t crc = 0;

    for (size_t i = 0; i < len; ++i) {
        uint8_t current = data[i];
        for (int b = 0; b < 8; ++b) {
            const bool mix = ((crc >> 7) ^ (current & 0x01)) != 0;
            crc <<= 1;
            if (mix) {
                crc ^= 0x07;
            }
            current >>= 1;
        }
    }

    return crc;
}

uint8_t TMC2209Driver::microstepsToMres(uint16_t microsteps) {
    switch (microsteps) {
        case 256: return 0;
        case 128: return 1;
        case  64: return 2;
        case  32: return 3;
        case  16: return 4;
        case   8: return 5;
        case   4: return 6;
        case   2: return 7;
        case   1: return 8;
        default:  return 4; // default to 16 microsteps
    }
}

void TMC2209Driver::flushRx() {
    while (uart_is_readable(mUartPort)) {
        (void)uart_getc(mUartPort);
    }
}

bool TMC2209Driver::readExact(uint8_t* dst, size_t len, uint32_t timeoutUs) {
    const absolute_time_t deadline = make_timeout_time_us(timeoutUs);
    size_t index = 0;

    while (index < len) {
        if (uart_is_readable(mUartPort)) {
            dst[index++] = uart_getc(mUartPort);
            continue;
        }

        if (absolute_time_diff_us(get_absolute_time(), deadline) <= 0) {
            return false;
        }
    }

    return true;
}
