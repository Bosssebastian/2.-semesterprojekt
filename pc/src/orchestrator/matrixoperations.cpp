#include "matrixoperations.h"

// The class functions uses radians, but takes input in degrees, which is translated here
double MatrixOperations::degToRad(double degree) {
	return (degree * (PI / 180));
}

double MatrixOperations::sRound(double val, double bound) {
	if (abs(val) < bound) {
		return 0;
	}
	else {
		return val;
	}
}

std::vector<std::vector<double>> MatrixOperations::roundMat(std::vector<std::vector<double>> mat) {
	for (int i = 0;i < mat.size();i++) {
		for (int j = 0;j < mat[i].size(); j++) {
			mat[i][j] = sRound(mat[i][j]);
		}
	}
	return mat;
}

std::vector<std::vector<double>> MatrixOperations::rotz(double theta) {
	std::vector<std::vector<double>> rot;
	rot.push_back({ cos(theta),-sin(theta),0 });
	rot.push_back({ sin(theta),cos(theta),0 });
	rot.push_back({ 0,0,1 });
	//std::cout << "rotz works\n";
	return rot;
}

std::vector<std::vector<double>> MatrixOperations::rotx(double theta) {
	std::vector<std::vector<double>> rot;
	rot.push_back({ 1,0,0 });
	rot.push_back({ 0,cos(theta),-sin(theta) });
	rot.push_back({ 0,sin(theta),cos(theta) });
	//std::cout << "rotx works\n";
	return rot;
}

std::vector<std::vector<double>> MatrixOperations::roty(double theta) {
	std::vector<std::vector<double>> rot;
	rot.push_back({ cos(theta),0,sin(theta) });
	rot.push_back({ 0,1,0 });
	rot.push_back({ -sin(theta),0,cos(theta) });
	//std::cout << "roty works\n";
	return rot;
}

std::vector<std::vector<double>> MatrixOperations::toTrans(std::vector<std::vector<double>> rot, std::vector<std::vector<double>> vec) {
	std::vector<std::vector<double>> transMat;
	for (int i = 0; i < 3; i++) {
		transMat.push_back({ rot[i][0],rot[i][1],rot[i][2],vec[i][0] });
	}
	transMat.push_back({ 0,0,0,1 }); // add shearing and scaling factors (constant)
	//std::cout << "toTrans works\n";
	return transMat;
}


std::vector<std::vector<double>> MatrixOperations::multMat(std::vector<std::vector<double>> mat1, std::vector<std::vector<double>> mat2) {
	std::vector<std::vector<double>> rsltmat;
	std::vector<double> newRow;
	for (int i = 0; i < mat1.size(); i++) { // run for each row in mat1
		newRow.clear();
		for (int j = 0; j < mat2[0].size(); j++) { // run for each element in of column in mat2
            double sum = 0.0;
			for (int k = 0; k < mat1[0].size(); k++) { // run for each element of row in mat1
				sum += mat1[i][k] * mat2[k][j];
			}
            newRow.push_back(sRound(sum));
		}
		rsltmat.push_back(newRow);
	}
	//std::cout << "multMat works\n";
	return rsltmat;
}

std::vector<std::vector<double>> MatrixOperations::inverseMat(std::vector<std::vector<double>> mat) {
	std::vector<std::vector<double>> inverseMat;
	std::vector<double> newRow;
	for (int i = 0;i < 3;i++) { // run for each column in rotation matrix
		newRow.clear();
		for (int j = 0;j < 3;j++) { // run for each row in rotation matrix
			newRow.push_back(mat[j][i]); // add column value to new row
		}
		inverseMat.push_back(newRow); // add new row to transformation matrix
	}
	std::vector<std::vector<double>> vec = { {-mat[0][3]},{-mat[1][3]},{-mat[2][3]} }; // create reverse vector
	std::vector<std::vector<double>> newVec = multMat(inverseMat, vec); // calculate transformation vector
	for (int i = 0;i < 3;i++) { // add transformation vector rows to inverse matrix rows
		inverseMat[i].push_back(newVec[i][0]);
	}
	inverseMat.push_back({ 0,0,0,1 }); // add shearing and scaling factors (constant)
	//std::cout << "inverseMat works\n";
	return inverseMat;
}