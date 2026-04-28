#pragma once
#include <vector>
#include <cmath>
class MatrixOperations
{
public:
	double degToRad(double); // Converts degrees to radians
    double sign(double); // Returns -1 or +1 depending on sign
    double sRound(double val, double bound = 0.0001); // Rounds to 0, if numeric value is lower than bound
    std::vector<std::vector<double>> roundMat(std::vector<std::vector<double>>); // Applies sRound to all elements of a matrix
	std::vector<std::vector<double>> rotz(double); // Creates rotation matrix around z-axis using radians
	std::vector<std::vector<double>> rotx(double); // Rotation matrix around x-axis
	std::vector<std::vector<double>> roty(double); // Rotation matrix around y-axis
	std::vector<std::vector<double>> toTrans(std::vector<std::vector<double>>  rot = { {1,0,0},{0,1,0},{0,0,1} }, std::vector<std::vector<double>> vec = { {0},{0},{0} }); // Converts rotation matrix and vector to transformation matrix
	std::vector<std::vector<double>> multMat(std::vector<std::vector<double>>, std::vector<std::vector<double>>); // Multiply two matrices
    std::vector<std::vector<double>> addMat(std::vector<std::vector<double>>, std::vector<std::vector<double>>); // Add elements of two matrices
    std::vector<std::vector<double>> factorMult(std::vector<std::vector<double>>, double); // Multiply all elemnts of a matrix with a factor
    std::vector<std::vector<double>> powerMat(std::vector<std::vector<double>>, double n = 0.5); // Each element of a matrix to the power of n
    double trace(std::vector<std::vector<double>>); // Adds diagonal elements of a nxn matrix
	std::vector<std::vector<double>> inverseTrans(std::vector<std::vector<double>>); // Invert direction of transformation matrix
    std::vector<std::vector<double>> inverseMatrix(std::vector<std::vector<double>>); // Transpose a matrix
    std::vector<std::vector<double>> diag(std::vector<std::vector<double>>); // Get diagonal values of a nxn matrix
    std::vector<std::vector<double>> maxMat(std::vector<std::vector<double>>, double limit = 0); // Compare each element of a vector with a limit and choose the greater
    double PI = 3.14159265358979323846; // Pi
    double PI_2 = 1.57079632679489661923; // Pi/2
    double PI_4 = 0.785398163397448309616; // Pi/4
    double rec_PI = 0.318309886183790671538; // 1/Pi
    std::vector<std::vector<double>> rot0 = {{1,0,0},{0,1,0},{0,0,1}}; // Zero degree rotaion
    std::vector<std::vector<double>> vec0 = {{0},{0},{0}}; // Zero translation vector
    std::vector<std::vector<double>> findBaseToCamTrans(double, bool = false, double rotationZ = 22.5); // Get transformation matrix from base frame to camera frame
    std::vector<std::vector<double>> getBaseToWorldTrans(); // Get transformation matrix from base frame to world frame
    void printMat(std::vector<std::vector<double>>); // Print matrix (for debugging)
};
