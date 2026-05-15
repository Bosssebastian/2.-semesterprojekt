#include "CurrentSensor.h"

#include <cstdio>

#include "hardware/adc.h"
#include "pico/time.h"

void CurrentSensor::setup() {
    adc_init();
    adc_gpio_init(CurrentSenseGpio);
    adc_select_input(CurrentSenseAdcInput);
    mStartMs = to_ms_since_boot(get_absolute_time());
    mNextSampleMs = mStartMs;
    mBatchStartMs = 0;
    mSampleCount = 0;
    mLatestAmps = 0.0f;
    mHasLatestSample = false;
    mSampleSequence = 0;
    mEventsEnabled = true;
}

void CurrentSensor::update() {
    const uint32_t nowMs = to_ms_since_boot(get_absolute_time());
    if (static_cast<int32_t>(nowMs - mNextSampleMs) < 0) {
        return;
    }

    if (mSampleCount == 0) {
        mBatchStartMs = mNextSampleMs - mStartMs;
    }

    mLatestAmps = readAmps();
    mHasLatestSample = true;
    ++mSampleSequence;
    mSamples[mSampleCount++] = mLatestAmps;
    mNextSampleMs += SamplePeriodMs;

    if (mSampleCount >= BatchSize) {
        flushBatch();
    }
}

bool CurrentSensor::hasLatestSample() const {
    return mHasLatestSample;
}

float CurrentSensor::latestAmps() const {
    return mLatestAmps;
}

uint32_t CurrentSensor::sampleSequence() const {
    return mSampleSequence;
}

void CurrentSensor::setEventsEnabled(bool enabled) {
    mEventsEnabled = enabled;
}

bool CurrentSensor::eventsEnabled() const {
    return mEventsEnabled;
}

uint16_t CurrentSensor::readRaw() const {
    adc_select_input(CurrentSenseAdcInput);
    return adc_read();
}

float CurrentSensor::readAmps() const {
    const float adcVoltage = (static_cast<float>(readRaw()) * AdcReferenceVoltage) / AdcMaxCount;
    return adcVoltage / (InaGain * ShuntResistanceOhms);
}

void CurrentSensor::flushBatch() {
    if (mSampleCount == 0 || !mEventsEnabled) {
        mSampleCount = 0;
        return;
    }

    std::printf("EVENT CURRENT %lu %lu",
                static_cast<unsigned long>(mBatchStartMs),
                static_cast<unsigned long>(SamplePeriodMs));

    for (uint8_t i = 0; i < mSampleCount; ++i) {
        std::printf(" %.3f", static_cast<double>(mSamples[i]));
    }

    std::printf("\n");
    mSampleCount = 0;
}
