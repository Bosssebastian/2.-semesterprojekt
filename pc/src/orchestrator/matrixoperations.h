#pragma once
#include <vector>
class MatrixOperations
{
public:
	double degToRad(double);
    double sRound(double val, double bound = 0.0001);
    std::vector<std::vector<double>> roundMat(std::vector<std::vector<double>>);
	std::vector<std::vector<double>> rotz(double);
	std::vector<std::vector<double>> rotx(double);
	std::vector<std::vector<double>> roty(double);
	std::vector<std::vector<double>> toTrans(std::vector<std::vector<double>>  rot = { {1,0,0},{0,1,0},{0,0,1} }, std::vector<std::vector<double>> vec = { {0},{0},{0} });
	std::vector<std::vector<double>> multMat(std::vector<std::vector<double>>, std::vector<std::vector<double>>);
	std::vector<std::vector<double>> inverseMat(std::vector<std::vector<double>>);
    double PI = 3.14159265358979323846;
    double PI_2 = 1.57079632679489661923;
    double PI_4 = 0.785398163397448309616;
    double rec_PI = 0.318309886183790671538;
    double rec_2_PI = 0.636619772367581343076;
};

