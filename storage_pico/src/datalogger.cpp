#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <ctime>

#include "datalogger.h"
#include "pathfinder.h"

dataLogger::dataLogger()
    : mSuccess(0),
      mIrregularities(0),
      mStart(0)
{
    mStoredPositions.clear();
    mStoredTargets.clear();
    mDeltaTime.clear();
    mDeltaMove.clear();
}

void dataLogger::reset()
{
    mStoredPositions.clear();
    mStoredTargets.clear();
    mDeltaTime.clear();
    mDeltaMove.clear();

    mSuccess = 0;
    mIrregularities = 0;
    mStart = 0;
}

void dataLogger::appendReading(int reading)
{
    mStoredPositions.push_back(reading);
}

void dataLogger::appendTarget(int target)
{
    mStoredTargets.push_back(target);
}

void dataLogger::checkIrregularities()
{
    pathFinder finder(0, 0, 8);

    for (int i = 1; i < mStoredPositions.size(); i++)
    {

        int now = mStoredPositions[i];
        int before = mStoredPositions[i-1];

        int directDistance = abs(before - now);
        //Using pathfinder to get circularDistance so 8-1 = 1
        int directCircularDistance = std::min(directDistance, 8 - finder.getCircularPosition(directDistance)); 
        

        if (directCircularDistance > 1)
        {
            mIrregularities += 1;
        }
        
    }
    
}

void dataLogger::timeTaskStart()
{
    //Perhaps it would be better to merge some of these functions, so every time we append target, timer starts
    //and every time we measure success we end timer and automatically measure time difference
    //But then the correct approach is to make anither method that does that, like startMeasurement and stopMeasurement

    float mStart = clock();

}

void dataLogger::timeTaskEnd()
{


    float slut = clock();
    
    float time = (slut - mStart) / CLOCKS_PER_SEC;
    mDeltaTime.push_back(time);
}

void dataLogger::calculateMovingDistance()
{
    for (int i = 1; i < mStoredTargets.size(); i++)
    {
        mDeltaMove.push_back(abs(mStoredTargets[i]-mStoredTargets[i-1]));
    }
    
}

void dataLogger::storeSuccess()
{
    mSuccess += 1;
}

void dataLogger::printData()
{

    calculateMovingDistance();
    checkIrregularities();

    std::cout << std::endl << std::endl << "PRINTING ALL DATA" << std::endl << std::endl;

    std::cout << "Irregularities: " << mIrregularities << std::endl;
    float average = 0;
    for (int i = 0; i < mDeltaMove.size(); i++)
    {
        average += mDeltaMove[i];
    }
    average = average / mDeltaMove.size();

    std::cout << "Average move distance: " << average << std::endl;
    std::cout << "Successes: " << mSuccess << std::endl << std::endl;

    //Printing mStoredTargets
    std::cout << "PRINTING mStoredTargets: " << std::endl;
    std::cout << "[ ";
    for (int i=0; i < mStoredTargets.size(); i++)
    {
        std::cout << mStoredTargets[i] << " ";
    }
    std::cout << "]" << std::endl << std::endl;

    //Printing mDeltaTime
    std::cout << "PRINTING mDeltaTime: " << std::endl;
    std::cout << "[ ";
    for (int i=0; i < mDeltaTime.size(); i++)
    {
        std::cout << mDeltaTime[i] << " ";
    }
    std::cout << "]" << std::endl << std::endl;

    //Printing mStoredPositions
    std::cout << "PRINTING mStoredPositions: " << std::endl;
    std::cout << "[ ";
    for (int i=0; i < mStoredPositions.size(); i++)
    {
        std::cout << mStoredPositions[i] << " ";
    }
    std::cout << "]" << std::endl << std::endl;

    //Printing mDeltaMove
    std::cout << "PRINTING mDeltaMove: " << std::endl;
    std::cout << "[ ";
    for (int i=0; i < mDeltaMove.size(); i++)
    {
        std::cout << mDeltaMove[i] << " ";
    }
    std::cout << "]" << std::endl << std::endl;
    
}