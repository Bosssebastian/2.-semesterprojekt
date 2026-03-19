#pragma once
#include <cstdint>
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/types.h"

namespace PinConfig {

// USB PC
constexpr uint32_t PC_BAUD = 115200;

// UART TMC2209
inline uart_inst_t* const TMC_UART_PORT = uart0;
constexpr uint8_t TMC_ADDRESS = 0x00;
constexpr uint TMC_UART_TX_PIN = 12;    //temp
constexpr uint TMC_UART_RX_PIN = 13;    //temp
constexpr uint32_t TMC_UART_BAUD = 115200;

// Stepper control pins
constexpr uint STEP_PIN = 20;
constexpr uint DIR_PIN = 21;
constexpr uint EN_PIN = 16;
constexpr uint DIAG_PIN = 17;

// Stall detection configuration
enum class StallDetectionMode : uint8_t { None, Uart, DiagInterrupt };

inline const char* toString(StallDetectionMode mode) {
    switch (mode) {
        case StallDetectionMode::None:
            return "None";
        case StallDetectionMode::Uart:
            return "Uart";
        case StallDetectionMode::DiagInterrupt:
            return "DiagInterrupt";
    }

    return "Unknown";
}
constexpr StallDetectionMode STALL_DETECTION_MODE = StallDetectionMode::DiagInterrupt;
constexpr uint32_t DIAG_IRQ_EVENT_MASK = GPIO_IRQ_EDGE_RISE;

}
