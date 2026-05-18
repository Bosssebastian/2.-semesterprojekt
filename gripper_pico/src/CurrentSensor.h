#pragma once

#include <cstdint>

class CurrentSensor {
public:
    void setup();
    void update();
    bool hasLatestSample() const;
    float latestAmps() const;
    uint32_t sampleSequence() const;
    void setEventsEnabled(bool enabled);
    bool eventsEnabled() const;

private:
    static constexpr uint32_t CurrentSenseGpio = 26;
    static constexpr uint32_t CurrentSenseAdcInput = 0;
    static constexpr float AdcReferenceVoltage = 3.3f;
    static constexpr float AdcMaxCount = 4095.0f;
    static constexpr float InaGain = 50.0f;
    static constexpr float ShuntResistanceOhms = 0.1f;
    static constexpr uint32_t SamplePeriodMs = 20;
    static constexpr uint8_t BatchSize = 5;

    uint32_t mStartMs = 0;
    uint32_t mNextSampleMs = 0;
    uint32_t mBatchStartMs = 0;
    float mSamples[BatchSize] = {};
    uint8_t mSampleCount = 0;
    float mLatestAmps = 0.0f;
    bool mHasLatestSample = false;
    uint32_t mSampleSequence = 0;
    bool mEventsEnabled = true;

    uint16_t readRaw() const;
    float readAmps() const;
    void flushBatch();
};
