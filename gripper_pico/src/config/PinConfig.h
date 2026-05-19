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
constexpr uint TMC_UART_TX_PIN = 16;
constexpr uint TMC_UART_RX_PIN = 17;
constexpr uint32_t TMC_UART_BAUD = 115200;

// UART
inline uart_inst_t* const UART_PORT = uart1;
constexpr uint UART_TX_PIN = 4;
constexpr uint UART_RX_PIN = 5;
constexpr uint32_t UART_BAUD = 115200;

// Stepper control pins
constexpr uint STEP_PIN = 20;
constexpr uint DIR_PIN = 21;
constexpr uint EN_PIN = 19;
constexpr uint DIAG_PIN = 18;

// Stall detection configuration
enum class StallDetectionMode : uint8_t { None, Uart, DiagInterrupt, Current };

inline const char* toString(StallDetectionMode mode) {
    switch (mode) {
        case StallDetectionMode::None:
            return "None";
        case StallDetectionMode::Uart:
            return "Uart";
        case StallDetectionMode::DiagInterrupt:
            return "DiagInterrupt";
        case StallDetectionMode::Current:
            return "Current";
    }

    return "Unknown";
}
constexpr StallDetectionMode STALL_DETECTION_MODE = StallDetectionMode::Uart;
constexpr uint32_t DIAG_IRQ_EVENT_MASK = GPIO_IRQ_EDGE_RISE;

}
