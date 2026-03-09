#pragma once
#include <cstdint>

namespace ParameterConfig {

constexpr uint32_t AXIS_START_SPEED_SPS = 500;
constexpr uint32_t AXIS_TARGET_SPEED_SPS = 2000;
constexpr uint32_t AXIS_ACCELERATION_SPS2 = 4000;

constexpr uint8_t DRIVER_HOLD_CURRENT = 8;   // Approx. 500 mA
constexpr uint8_t DRIVER_RUN_CURRENT = 8;    // Approx. 500 mA
constexpr uint8_t DRIVER_HOLD_DELAY = 6;
constexpr uint16_t DRIVER_MICROSTEPS = 16;
constexpr bool DRIVER_SPREAD_CYCLE_ENABLED = false;
constexpr uint32_t DRIVER_PWM_THRESHOLD = 0;
constexpr uint32_t DRIVER_COOL_THRESHOLD = 0x000fffff;
constexpr uint8_t DRIVER_STALL_THRESHOLD = 10;

constexpr int32_t GRIPPER_CLOSE_STEPS = 6000;                   // Steps to fully close the gripper. 
constexpr int32_t GRIPPER_OPEN_RELEASE_STEPS = 300;             // Steps to attempt if stall is detected during open (release)
constexpr int32_t GRIPPER_OPEN_HOME_STEPS = 7000;               // Steps to fully open (home) the gripper. Should be higher than the sum of close and release steps to ensure full opening.
constexpr int32_t GRIPPER_OPEN_RETRY_RELEASE_STEPS = 300;       // Additional steps to attempt if stall is detected during open (release)
constexpr int32_t GRIPPER_OPEN_RETRY_HOME_STEPS = 7000;         // Additional steps to attempt if stall is detected during open (release or home)

}
