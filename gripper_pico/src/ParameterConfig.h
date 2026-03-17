#pragma once
#include <cstdint>

namespace ParameterConfig {

constexpr uint32_t AXIS_START_SPEED_SPS = 1000;
constexpr uint32_t AXIS_TARGET_SPEED_SPS = 150000;
constexpr uint32_t AXIS_ACCELERATION_SPS2 = 100000;

constexpr uint8_t DRIVER_HOLD_CURRENT = 12;
constexpr uint8_t DRIVER_RUN_CURRENT = 24;
constexpr uint8_t DRIVER_HOLD_DELAY = 8;
constexpr uint16_t DRIVER_MICROSTEPS = 8;
constexpr bool DRIVER_SPREAD_CYCLE_ENABLED = true;
constexpr uint32_t DRIVER_PWM_THRESHOLD = 0;
constexpr uint32_t DRIVER_COOL_THRESHOLD = 0x000fffff;
constexpr uint8_t DRIVER_STALL_THRESHOLD = 10;

constexpr int32_t GRIPPER_CLOSE_STEPS = 230000;                   // Steps to fully close the gripper. 
constexpr int32_t GRIPPER_OPEN_STEPS = 230000;                    // Steps to fully open the gripper.
}
