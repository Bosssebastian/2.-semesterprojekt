#pragma once
#include <cstdint>
#include "pico/types.h"

class PwmOutput {
public:
    PwmOutput() = default;

    void begin(uint pin);
    void setFrequency(float frequencyHz);
    void start();
    void stop();
    uint32_t takeWrapCount();

private:
    uint mPin = 0;
    uint mSlice = 0;
    uint16_t mWrap = 0;
    volatile uint32_t mWrapCount = 0;

    static void pwmIrqRouter();
    void handleWrap();

    static constexpr uint MaxPwmSliceCount = 8;
    static PwmOutput* mOwners[MaxPwmSliceCount];
    static bool mIrqInstalled;
};
