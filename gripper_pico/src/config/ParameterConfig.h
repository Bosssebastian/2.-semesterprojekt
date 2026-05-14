#pragma once
#include <cstdint>

namespace ParameterConfig {

// Gripper profile: keep speed and acceleration modest so the motor keeps
// torque under load and StallGuard can detect the end stop cleanly.
constexpr uint32_t AXIS_START_SPEED_SPS = 1000;//1000;
constexpr uint32_t AXIS_TARGET_SPEED_SPS = 100000;//150000;
constexpr uint32_t AXIS_ACCELERATION_SPS2 = 400000;//100000;

constexpr uint8_t DRIVER_HOLD_CURRENT = 12;
constexpr uint8_t DRIVER_RUN_CURRENT = 28;
constexpr uint8_t DRIVER_HOLD_DELAY = 8;
constexpr uint16_t DRIVER_MICROSTEPS = 8;
constexpr bool DRIVER_SPREAD_CYCLE_ENABLED = true;
constexpr uint32_t DRIVER_PWM_THRESHOLD = 0;
constexpr uint32_t DRIVER_COOL_THRESHOLD = 0x000fffff;
// SG_RESULT is compared against DRIVER_STALL_THRESHOLD * 2. Free motion is
// around 300+, while loaded stall/contact falls through roughly 200 and below.
constexpr uint8_t DRIVER_STALL_THRESHOLD = 100;
constexpr uint32_t DRIVER_STALL_ARM_DELAY_US = 100000;
constexpr uint32_t DRIVER_STALL_ARM_STEPS = 1000;
constexpr uint32_t DRIVER_STALL_POLL_INTERVAL_US = 5000;
constexpr uint32_t DRIVER_STALL_LOG_INTERVAL_US = 50000;
constexpr uint8_t DRIVER_STALL_CONSECUTIVE_SAMPLES = 4;
constexpr uint16_t DRIVER_STALL_PRIME_MIN_SG_RESULT = 40;
constexpr uint16_t DRIVER_STALL_PRIME_COMPARE_MARGIN = 20;
constexpr uint16_t DRIVER_STALL_PRIME_DROP_DELTA = 8;
constexpr uint32_t DRIVER_STALL_BRAKE_MARGIN_STEPS = 1000;

constexpr int32_t GRIPPER_CLOSE_STEPS = 2300000;                   // Steps to fully close the gripper. 
constexpr int32_t GRIPPER_OPEN_STEPS = 800000;                     // Normal open travel before the homing/stall leg.
constexpr int32_t GRIPPER_RESET_OPEN_STEPS = 5000000;              // Homing travel used to guarantee a stall at the open limit.
constexpr int32_t GRIPPER_RESET_FORWARD_STEPS = 500000;              // Steps forward after homing against the open limit.
}
