#include "PwmOutput.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"

namespace {

constexpr uint16_t DefaultPwmWrap = 999;
constexpr uint16_t MaxPwmWrap = 65535;
constexpr float MinPwmFrequencyHz = 1.0f;
constexpr float MinPwmClockDiv = 1.0f;
constexpr float MaxPwmClockDiv = 255.0f;

}

PwmOutput* PwmOutput::mOwners[PwmOutput::MaxPwmSliceCount] = {};
bool PwmOutput::mIrqInstalled = false;

void PwmOutput::begin(uint pin) {
    mPin = pin;

    gpio_init(mPin);
    gpio_set_dir(mPin, GPIO_OUT);
    gpio_put(mPin, 0);
    gpio_set_function(mPin, GPIO_FUNC_PWM);

    mSlice = pwm_gpio_to_slice_num(mPin);
    if (mSlice < MaxPwmSliceCount) {
        mOwners[mSlice] = this;
    }

    if (!mIrqInstalled) {
        irq_set_exclusive_handler(PWM_IRQ_WRAP, &PwmOutput::pwmIrqRouter);
        irq_set_enabled(PWM_IRQ_WRAP, true);
        mIrqInstalled = true;
    }

    pwm_set_gpio_level(mPin, 0);
    pwm_set_enabled(mSlice, false);
    pwm_set_irq_enabled(mSlice, false);
}

void PwmOutput::setFrequency(float frequencyHz) {
    if (frequencyHz < MinPwmFrequencyHz) {
        frequencyHz = MinPwmFrequencyHz;
    }

    const uint32_t systemClockHz = clock_get_hz(clk_sys);
    uint32_t wrap = DefaultPwmWrap;
    float clockDiv = static_cast<float>(systemClockHz) / (frequencyHz * static_cast<float>(wrap + 1u));

    if (clockDiv > MaxPwmClockDiv) {
        wrap = MaxPwmWrap;
        clockDiv = static_cast<float>(systemClockHz) / (frequencyHz * static_cast<float>(wrap + 1u));
    }

    if (clockDiv < MinPwmClockDiv) {
        clockDiv = MinPwmClockDiv;
    } else if (clockDiv > MaxPwmClockDiv) {
        clockDiv = MaxPwmClockDiv;
    }

    mWrap = static_cast<uint16_t>(wrap);
    pwm_set_wrap(mSlice, mWrap);
    pwm_set_clkdiv(mSlice, clockDiv);
}

void PwmOutput::start() {
    mWrapCount = 0;
    pwm_clear_irq(mSlice);
    pwm_set_irq_enabled(mSlice, true);
    pwm_set_gpio_level(mPin, (mWrap + 1u) / 2u);
    pwm_set_enabled(mSlice, true);
}

void PwmOutput::stop() {
    mWrapCount = 0;
    pwm_set_irq_enabled(mSlice, false);
    pwm_set_enabled(mSlice, false);
    pwm_clear_irq(mSlice);
    pwm_set_gpio_level(mPin, 0);
}

uint32_t PwmOutput::takeWrapCount() {
    const uint32_t interruptState = save_and_disable_interrupts();
    const uint32_t wrapCount = mWrapCount;
    mWrapCount = 0;
    restore_interrupts(interruptState);
    return wrapCount;
}

void PwmOutput::pwmIrqRouter() {
    const uint32_t irqMask = pwm_get_irq_status_mask();

    for (uint slice = 0; slice < MaxPwmSliceCount; ++slice) {
        if ((irqMask & (1u << slice)) == 0u) {
            continue;
        }

        pwm_clear_irq(slice);

        PwmOutput* owner = mOwners[slice];
        if (owner != nullptr) {
            owner->handleWrap();
        }
    }
}

void PwmOutput::handleWrap() {
    ++mWrapCount;
}
