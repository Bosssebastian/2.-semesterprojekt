#ifndef DATALOGGER_H

#define DATALOGGER_H

#include <iostream>
#include <vector>
#include <cmath>


class dataLogger
{
    public:
        dataLogger(); //member variable of successfulReadings(vector) irregularities(integer), targets(vector), success(integer) deltaTime(float)


        void appendReading(); //Appends 1,2,3,4,5,6,7,8 if reading is different from before and not -1 (code already exists) (append to vector)
        void checkIrregularity(); //For each appendReading, we check if there is an irregularity (check last element of vector m)
        //Method should be able to append order and count the number of irregularities, 1,2,3,4,5,6,7,8 if change > +-1 that is one irregularity
        //COMMENT: If we keep a vector over all targets and readings, well then we do not need to check for irregularities every time
        //COMMENT: 

        void timeTaskStart();
        void timeTaskEnd();
        //Method should be able to time from target aquired to target reached, two methods, 
        //one start and one end, the difference between them is time for rotation

        void storeMovingDistance(); // if it need to go from 8-4, that is a distance of 4, each possible moving distance is stored in vector
        //Should take random assigned number into consideration, longer tasks takes 
        //longer time, we just need to check that the distribution of tasks roughly match
        //COMMENT: We do not need to calculate this every time, since we have the vector of all targets

        void storeSuccess();
        //Method here should be able to append every success.

        void writeToFile();
        //Method should be able to write to .txt 

    private:
    /*
        mSuccess;
        mIrregularities;
        mDeltaTime;
        mMovingDistance;
    */
};

#endif