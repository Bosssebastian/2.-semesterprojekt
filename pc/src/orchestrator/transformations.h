#pragma once
#include "matrixoperations.h"
class Transformations
{
public:
    Transformations();
    Transformations(double,bool=false,double=22.5); // set base to camera transformation
    std::vector<std::vector<double>> getBaseToCamTrans();
    std::vector<std::vector<double>> getCamToBaseTrans();
    std::vector<double> getMovementVec(std::vector<std::vector<double>>, bool testWithoutCam = false);
    ~Transformations(){};
private:
    MatrixOperations matop;
    std::vector<std::vector<double>> baseToCamTransform;
    std::vector<std::vector<double>> camToBaseTransform;
};