#pragma once
#include <cstdint>

namespace ParameterConfig {

// Debug profile that still produces visible motion.
constexpr uint32_t AXIS_START_SPEED_SPS = 500;//1000;
constexpr uint32_t AXIS_TARGET_SPEED_SPS = 50000;//150000;
constexpr uint32_t AXIS_ACCELERATION_SPS2 = 10000;//100000;

constexpr uint8_t DRIVER_HOLD_CURRENT = 12;
constexpr uint8_t DRIVER_RUN_CURRENT = 24;
constexpr uint8_t DRIVER_HOLD_DELAY = 8;
constexpr uint16_t DRIVER_MICROSTEPS = 8;
constexpr bool DRIVER_SPREAD_CYCLE_ENABLED = false;
constexpr uint32_t DRIVER_PWM_THRESHOLD = 0;
constexpr uint32_t DRIVER_COOL_THRESHOLD = 0x000fffff;
constexpr uint8_t DRIVER_STALL_THRESHOLD = 45;
constexpr uint32_t DRIVER_STALL_ARM_DELAY_US = 100000;
constexpr uint32_t DRIVER_STALL_ARM_STEPS = 1000;
constexpr uint32_t DRIVER_STALL_POLL_INTERVAL_US = 2000;
constexpr uint8_t DRIVER_STALL_CONSECUTIVE_SAMPLES = 3;
constexpr uint16_t DRIVER_STALL_PRIME_MIN_SG_RESULT = 40;
constexpr uint16_t DRIVER_STALL_PRIME_COMPARE_MARGIN = 20;
constexpr uint16_t DRIVER_STALL_PRIME_DROP_DELTA = 8;
constexpr uint32_t DRIVER_STALL_BRAKE_MARGIN_STEPS = 1000;

constexpr int32_t GRIPPER_CLOSE_STEPS = 230000;                   // Steps to fully close the gripper. 
constexpr int32_t GRIPPER_OPEN_STEPS = 230000;                    // Steps to fully open the gripper.
}
