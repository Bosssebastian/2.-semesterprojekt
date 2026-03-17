#include <stdio.h>

#include "Gripper.h"
#include "ParameterConfig.h"
#include "PinConfig.h"
#include "StepperAxis.h"
#include "TMC2209Driver.h"
#include "TestInterface.h"

#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    TMC2209Driver driver(PinConfig::TMC_UART_PORT, PinConfig::TMC_UART_BAUD, PinConfig::TMC_ADDRESS);
    driver.begin();
    const bool driverConfigured =
        driver.setCurrent(ParameterConfig::DRIVER_HOLD_CURRENT, ParameterConfig::DRIVER_RUN_CURRENT, ParameterConfig::DRIVER_HOLD_DELAY) &&
        driver.setMicrosteps(ParameterConfig::DRIVER_MICROSTEPS) &&
        driver.enableSpreadCycle(ParameterConfig::DRIVER_SPREAD_CYCLE_ENABLED) &&
        driver.setPwmThreshold(ParameterConfig::DRIVER_PWM_THRESHOLD) &&
        driver.configureStallGuard(ParameterConfig::DRIVER_COOL_THRESHOLD, ParameterConfig::DRIVER_STALL_THRESHOLD);

    StepperAxis axis(&driver);
    axis.begin();

    Gripper gripper(axis);
    gripper.begin();

    TestInterface testInterface(driver, axis, gripper);
    testInterface.begin(driverConfigured);

    while (true) {
        axis.update();
        gripper.update();
        testInterface.update();

        tight_loop_contents();
    }
}
