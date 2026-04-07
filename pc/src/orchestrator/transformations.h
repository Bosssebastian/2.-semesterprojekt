#pragma once
#include "matrixoperations.h"
class Transformations
{
public:
    Transformations();
    Transformations(double,bool=false,double=30.); // set base to camera transformation
    std::vector<std::vector<double>> getBaseToCamTrans();
    std::vector<std::vector<double>> getCamToBaseTrans();
    std::vector<double> getMovementVec(std::vector<std::vector<double>>);
    ~Transformations(){};
private:
    MatrixOperations matop;
    std::vector<std::vector<double>> baseToCamTransform;
    std::vector<std::vector<double>> camToBaseTransform;
};