#pragma once
#include "matrixoperations.h"
#include "orientation.h"
class Transformations
{
public:
    Transformations();
    Transformations(double,bool=false,double=22.5); // Set base to camera transformation
    std::vector<std::vector<double>> getBaseToCamTrans(); // Get transformation matrix from base frame to camera frame
    std::vector<std::vector<double>> getCamToBaseTrans(); // Get transformation matrix from camera frame to base frame
    std::vector<double> getMovementVec(std::vector<std::vector<double>>, double toolAngleRot = -22.5, bool testWithoutCam = false); // Get vector accepted by ur_rtde moveL function
    ~Transformations(){};
private:
    MatrixOperations matop;
    std::vector<std::vector<double>> baseToCamTransform;
    std::vector<std::vector<double>> camToBaseTransform;
};