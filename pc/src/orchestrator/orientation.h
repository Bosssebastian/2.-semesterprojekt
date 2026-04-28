#pragma once
#include "matrixoperations.h"
class Orientation
{
public:
    Orientation(){};
    double norm(std::vector<double>);
    std::vector<std::vector<double>> getRotMat(std::vector<double>, double);
    std::vector<double> rotMat2rv(std::vector<std::vector<double>>);
    std::vector<std::vector<double>> compose(std::vector<std::vector<double>>, std::vector<std::vector<double>>);
    std::vector<double> getAngleAxisVector(double rotationZ = 22.5);
    ~Orientation(){};
private:
    MatrixOperations matop;
};