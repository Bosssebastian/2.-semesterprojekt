//DrejeBaenkDecoder.cpp
#include "drejeBaenkDecoder.h"
#include <array>
#include "pico/stdlib.h"


drejeBaenkDecoder::drejeBaenkDecoder(ldrSensors& ldr1, ldrSensors& ldr2, ldrSensors& ldr3, ldrSensors& ldr4)
: _ldr1(ldr1), _ldr2(ldr2), _ldr3(ldr3), _ldr4(ldr4)
{

}

int drejeBaenkDecoder::nextPosition(int pos) const
{
    if (pos == 8) 
    {
        return 1;
    }
    return pos+1;
}

int drejeBaenkDecoder::preveousPosition(int pos) const
{
    if (pos == 1) 
    {
        return 8;
    }
    return pos-1;
}

int drejeBaenkDecoder::circularPosition(int a, int b) const
{

    int direct = abs(b-a);
    int directCircular = std::min(direct, 8 - direct);
    return directCircular;
}

int drejeBaenkDecoder::decipherExpected(float direction)
{
    int raw = decipherRaw();

    if (raw == -1)
    {
        return mLastAccepted;
    }

    if (mLastAccepted == -1)
    {
        mLastAccepted = raw;
        mLastRejected = -1;
        mRejectedRepeatCount = 0;
        mLastAcceptedTime = to_ms_since_boot(get_absolute_time());
        return mLastAccepted;
    }

    if (direction == 0)
    {
        return mLastAccepted;
    }

    if (raw == mLastAccepted)
    {
        return mLastAccepted;
    }

    int expected;

    if (direction > 0)
    {
        expected = nextPosition(mLastAccepted);

    }
    else
    {
        expected = preveousPosition(mLastAccepted);
    }

    //If the reading that we get is actually the reading that we expect
    if (raw == expected)
    {
        mLastAccepted = raw;
        mLastRejected = -1;
        mRejectedRepeatCount = 0;
        mLastAcceptedTime = to_ms_since_boot(get_absolute_time());
        return mLastAccepted;
    }

    if (raw == mLastRejected)
    {
        mRejectedRepeatCount++;
    }
    else
    {
        mLastRejected = raw;
        mRejectedRepeatCount = 1;
    }
    
    int now = to_ms_since_boot(get_absolute_time());
    int timeSInceAccepted = now - mLastAcceptedTime;

    const int REQUIRED_REPEAT_COUNT = 3;
    const int MAX_EXPECTED_STEP_TIME_MS = 700; //Should be defined outside 

    //if we need to accept the value, manages to continue if we completely missed a number

    if (mRejectedRepeatCount >= REQUIRED_REPEAT_COUNT ||
         timeSInceAccepted > MAX_EXPECTED_STEP_TIME_MS)
         {
            mLastAccepted = raw;
            mLastRejected = -1;
            mRejectedRepeatCount = 0;
            mLastAcceptedTime = now;
            return mLastAccepted;
         }
    
        return mLastAccepted;

}

int drejeBaenkDecoder::decipherRaw()
{
    std::array<int, 4> bits = readBitsSynced();

    for (int i=0; i < CIFFERTABEL.size(); i++)
    {
        if(CIFFERTABEL[i] == bits)
    {
        return i+1;
        
    }
    }
    return -1;
    
}



std::array<int, 4> drejeBaenkDecoder::readBits()
{
    return std::array<int, 4>
    {
        _ldr1.interpretValue(),
        _ldr2.interpretValue(),
        _ldr3.interpretValue(),
        _ldr4.interpretValue()
    };
}

std::array<int, 4> drejeBaenkDecoder::readBitsSynced()
{
    const int samples = 5;

    float total1 = 0;
    float total2 = 0;
    float total3 = 0;
    float total4 = 0;
    
    for (int i = 0; i < samples; i++)
    {
        total1 += _ldr1.readRaw();
        total2 += _ldr1.readRaw();
        total3 += _ldr1.readRaw();
        total4 += _ldr1.readRaw();

        sleep_ms(2);
    }

    float avg1 = total1 / samples;
    float avg2 = total1 / samples;
    float avg3 = total1 / samples;
    float avg4 = total1 / samples;

    return 
    std::array<int, 4>
    {
        _ldr1.interpretRawValue(avg1),
        _ldr2.interpretRawValue(avg2),
        _ldr3.interpretRawValue(avg3),
        _ldr4.interpretRawValue(avg4),
    };

}

void drejeBaenkDecoder::saveAllValues()
{
    _ldr1.saveValue();
    _ldr2.saveValue();
    _ldr3.saveValue();
    _ldr4.saveValue();
}


void drejeBaenkDecoder::timeBasedCalibration(DcMotor& motor, uint32_t calibrationTimeMs)
{

    //Clear values, but we only do one calibration at the beginning


    uint32_t startTime = to_ms_since_boot(get_absolute_time());

    motor.forwards();

    while ((to_ms_since_boot(get_absolute_time()) - startTime) < calibrationTimeMs)
    {
        saveAllValues();
        sleep_ms(2);
    }
    motor.stop();

    motor.backwards();

    while ((to_ms_since_boot(get_absolute_time()) - startTime) < calibrationTimeMs * 2)
    {
        saveAllValues();
        sleep_ms(2);
    }
    motor.stop();

    _ldr1.totalCalibration();
    _ldr2.totalCalibration();
    _ldr3.totalCalibration();
    _ldr4.totalCalibration();

}
