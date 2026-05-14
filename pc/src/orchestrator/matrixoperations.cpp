#include "matrixoperations.h"
#include <iostream> // for debugging

// The class functions uses radians, but takes input in degrees, which is translated here
double MatrixOperations::degToRad(double degree) {
	return (degree * (PI / 180));
}

double MatrixOperations::sign(double val) {
	return std::copysign(1.0,val);
	/*if (val < 0) {
		return (-1);
	}
	else {
		return (1);
	}*/
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
	for (unsigned int i = 0;i < mat.size();i++) {
		for (unsigned int j = 0;j < mat[i].size(); j++) {
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
	transMat.push_back({ 0.0,0.0,0.0,1.0 }); // add shearing and scaling factors (constant)
	//std::cout << "toTrans works\n";
	return transMat;
}

/*
std::vector<std::vector<double>> MatrixOperations::multMat(std::vector<std::vector<double>> mat1, std::vector<std::vector<double>> mat2) {
	//mat1 = roundMat(mat1);
	//mat2 = roundMat(mat2);
	std::vector<std::vector<double>> rsltmat;
	std::vector<double> newRow;
	for (unsigned int i = 0; i < mat1.size(); i++) { // run for each row in mat1
		newRow.clear();
		for (unsigned int j = 0; j < mat2[0].size(); j++) { // run for each element in of column in mat2
			double sum = 0.0;
			for (unsigned int k = 0; k < mat1[0].size(); k++) { // run for each element of row in mat1
				sum += mat1[i][k] * mat2[k][j];
			}
			newRow.push_back(sRound(sum));
		}
		rsltmat.push_back(newRow);
	}
	//std::cout << "multMat works\n";
	return rsltmat;
}
*/
std::vector<std::vector<double>> MatrixOperations::multMat(std::vector<std::vector<double>> mat1, std::vector<std::vector<double>> mat2)
{
    std::vector<std::vector<double>> rsltmat;

    for (size_t i = 0; i < mat1.size(); ++i) {

        std::vector<double> newRow;

        for (size_t j = 0; j < mat2[0].size(); ++j) {

            double sum = 0.0;

            for (size_t k = 0; k < mat1[0].size(); ++k) {
                sum += mat1[i][k] * mat2[k][j];
            }

            newRow.push_back(sum);
        }

        rsltmat.push_back(newRow);
    }

    return rsltmat;
}
std::vector<std::vector<double>> MatrixOperations::addMat(std::vector<std::vector<double>> mat1, std::vector<std::vector<double>> mat2) {
	std::vector<std::vector<double>> sumMat;
	for (size_t i = 0; i < mat1.size(); ++i) {
		std::vector<double> sumVec;
		for (size_t j = 0; j < mat1[i].size(); ++j) {
			sumVec.push_back(mat1[i][j] + mat2[i][j]);
		}
		sumMat.push_back(sumVec);
	}
	return sumMat;
}

std::vector<std::vector<double>> MatrixOperations::factorMult(std::vector<std::vector<double>> mat1, double factor) {
	std::vector<std::vector<double>> resMat;
	for (size_t i = 0; i < mat1.size(); ++i) {
		std::vector<double> resVec;
		for (size_t j = 0; j < mat1[i].size(); ++j) {
			resVec.push_back(mat1[i][j] * factor);
		}
		resMat.push_back(resVec);
	}
	return resMat;
}

std::vector<std::vector<double>> MatrixOperations::powerMat(std::vector<std::vector<double>> mat, double n) { // Each element of a matrix to the power of n
	for (size_t i = 0; i < mat.size(); ++i) {
		for (size_t j = 0; j < mat[i].size(); ++j) {
			mat[i][j] = pow(mat[i][j], n);
		}
	}
	return mat;
}

double MatrixOperations::trace(std::vector<std::vector<double>> matrix) {
	double res = 0;
	for (size_t i = 0; i < matrix.size(); ++i) {
		if (matrix[i].size() != matrix.size()) { // if rows != columns, error detected, return -1
			return -1;
		}
		res += matrix[i][i];
	}
	return res;
}

std::vector<std::vector<double>> MatrixOperations::transposeMatrix(std::vector<std::vector<double>> mat) {
	std::vector<std::vector<double>> transposeMat;
	std::vector<double> newRow;
	for (unsigned int i = 0; i < mat.size(); i++) { // run for each column in rotation matrix
		newRow.clear();
		for (unsigned int j = 0; j < mat[i].size(); j++) { // run for each row in rotation matrix
			newRow.push_back(mat[j][i]); // add column value to new row
		}
		transposeMat.push_back(newRow); // add new row to transformation matrix
	}
	return transposeMat;
}

std::vector<std::vector<double>> MatrixOperations::transposeTrans(std::vector<std::vector<double>> mat) {
	std::vector<std::vector<double>> transposeMat;
	std::vector<double> newRow;
	for (int i = 0;i < 3;i++) { // run for each column in rotation matrix
		newRow.clear();
		for (int j = 0;j < 3;j++) { // run for each row in rotation matrix
			newRow.push_back(mat[j][i]); // add column value to new row
		}
		transposeMat.push_back(newRow); // add new row to transformation matrix
	}
	std::vector<std::vector<double>> revVec = { {-mat[0][3]},{-mat[1][3]},{-mat[2][3]} }; // create reverse vector
	std::vector<std::vector<double>> newVec = multMat(transposeMat, revVec); // calculate transformation vector
	for (int i = 0;i < 3;i++) { // add transformation vector rows to transposed matrix rows
		transposeMat[i].push_back(newVec[i][0]);
	}
	transposeMat.push_back({ 0,0,0,1 }); // add shearing and scaling factors (constant)
	//std::cout << "transposeMat works\n";
	return transposeMat;
}

std::vector<std::vector<double>> MatrixOperations::diag(std::vector<std::vector<double>> mat) {
	std::vector<std::vector<double>> resMat;
	if (mat[0].size() > 1) { // get diagonal values
		for (unsigned int i = 0; i < mat.size(); i++) {
			resMat.push_back({ mat[i][i] });
		}
	}
	else { // set diagonal values (not yet functional due to not being used)
		std::vector<double> newRow;
		for (unsigned int i = 0; i < mat.size(); i++) {
			newRow.push_back(-1);
		}
		resMat.push_back(newRow);
	}
	return resMat;
}

std::vector<std::vector<double>> MatrixOperations::maxMat(std::vector<std::vector<double>> mat, double limit) {
	for (unsigned int i = 0; i < mat.size(); i++) {
		for (unsigned int j = 0; j < mat[i].size(); j++) {
			if (mat[i][j] < limit) {
				mat[i][j] = limit;
			}
		}
	}
	return mat;
}

std::vector<std::vector<double>> MatrixOperations::findBaseToCamTrans(double upZ, bool inwardsY, double rotationZ) {
	double totalZ = (50. * upZ - 32.) / 1000.; // add 50mm for each hole from table surface (first half hole does not count), -32 to account for raised base plane
	double totalY = -(600. + 50. / 2.0 + 12.); // length from base center first hole before sidebar middle, plus half the distance between two holes, plus width of camera mount
	double cameraOffset = (42. - 29.) + (29. / 2.); // onboard mount width, plus distance to middle of camera lens
	std::vector<std::vector<double>> rot;
	std::vector<std::vector<double>> bToWTrans = getBaseToWorldTrans(); // transformation matrix from base frame to world frame
	//printMat(bToWTrans);
	std::cout << "\n";
	if (inwardsY) { // add/subtract cameraOffset if camera is mounted inwards/outwards, and rotate to match orientation (using euler angles)
		totalY += cameraOffset;
		//rot = multMat(rotz(degToRad(rotationZ)),roty(PI));
		rot = roty(PI);
	}
	else {
		totalY -= cameraOffset;
		//rot = multMat(rotz(degToRad(rotationZ)),rotx(PI));
		rot = rotx(PI);
	}
	totalY /= 1000.; // convert to m
	double totalX = -(425. - 158. - 92.) / 1000.; // distance from base center to table side, minus mounting arm link to camera center, minus distance from sidebar to mounting arm link
	std::vector<std::vector<double>> vec = { {totalX},{totalY},{totalZ} }; // vector from world frame origin to camera frame origin
	//printMat(vec);
	//std::cout << "\n";
	std::vector<std::vector<double>> wToCTrans = toTrans(rot, vec); // transformation matrix from world frame to camera frame
	//printMat(wToCTrans);
	std::vector<std::vector<double>> bToCTrans = multMat(bToWTrans, wToCTrans); // transformation matrix from base frame to camera frame
	//printMat(bToCTrans);
	return {{-0.9239, -0.3827, 0.0, 0.0974},{-0.3827, 0.9239, 0.0, -0.6924},{0.0, 0.0, -1.0000, 0.6680},{0.0, 0.0, 0.0, 1.0000}}; //bToCTrans;
}

std::vector<std::vector<double>> MatrixOperations::getBaseToWorldTrans() {
	std::vector<std::vector<double>> rot = rotz(degToRad(22.5));
	return toTrans(rot, vec0);
}


void MatrixOperations::printMat(std::vector<std::vector<double>> matrix) { // for debugging
	for (unsigned int i = 0; i < matrix.size();i++) {
		for (unsigned int j = 0;j < matrix[i].size();j++) {
			std::cout << matrix[i][j] << " ";
		}
		std::cout << std::endl;
	}
}