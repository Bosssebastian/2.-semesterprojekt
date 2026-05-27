#pragma once
#include "matrixoperations.h"
class Orientation
{
public:
    Orientation(){};
    double norm(std::vector<double>);
    std::vector<std::vector<double>> getRotMat(std::vector<double>, double);
    //std::vector<double> rotMat2rv(std::vector<std::vector<double>>); // Outdated
    std::vector<double> rotMat2rv2(std::vector<std::vector<double>>);
    std::vector<std::vector<double>> compose(std::vector<std::vector<double>>, std::vector<std::vector<double>>);
    std::vector<double> getAngleAxisVector(double rotationZ = 0.39269908169872415480783042290994, bool deg = false);
    ~Orientation(){};
private:
    MatrixOperations matop;
};