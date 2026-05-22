#include "orientation.h"

double Orientation::norm(std::vector<double> v) {
    double sum = 0;
    for (unsigned int i = 0; i < v.size(); i++) {
        sum += v[i] * v[i];
    }
    return sqrt(sum);
}

std::vector<std::vector<double>> Orientation::getRotMat(std::vector<double> axis, double angle) {
    std::vector<double> unitK;
    double normAxis = norm(axis);
    for (size_t i = 0; i < axis.size(); ++i) {
        unitK.push_back(axis[i] / normAxis);
    } // ensure unit vector
    std::vector<std::vector<double>> skewK = { {0,-unitK[2],unitK[1]}, {unitK[2],0,-unitK[0]}, {-unitK[1],unitK[0],0} }; // skew symmetric matrix, K
    std::vector<std::vector<double>> rotMLHS = matop.addMat(matop.rot0, matop.factorMult(skewK, sin(angle))); // eye(3) + sin(angle)*K
    std::vector<std::vector<double>> kSq = matop.multMat(skewK, skewK); // K*K
    std::vector<std::vector<double>> rotMRHS = matop.factorMult(kSq, (1.0 - cos(angle))); // (1-cos(angle))*(K*K)

    return matop.addMat(rotMLHS, rotMRHS);
}

/*std::vector<double> Orientation::rotMat2rv(std::vector<std::vector<double>> rotmat) {
    double angle = acos((matop.trace(rotmat) - 1.) / 2.);
    if (abs(angle) < 0.0001) { // Check for and handle near-zero rotation
        return { 0,0,0 };
    }
    std::vector<std::vector<double>> axis;
    if (abs(angle - matop.PI) < 0.0001) { // Check for and handle near-180 degrees (alternative extraction)
        // extract axis from symmetric part of R
        std::vector<std::vector<double>> B = matop.factorMult(matop.addMat(rotmat, matop.transposeMatrix(rotmat)), 1. / 2.); // (R+R')/2
        std::vector<std::vector<double>> BRHS = matop.factorMult(matop.rot0, -cos(angle)); // -eye(3)*cos(angle)
        B = matop.addMat(B, BRHS);
        B = matop.factorMult(B, 1. / (1. - cos(angle))); // B/(1-cos(angle))

        // diagonal gives squared axis components
        axis = matop.powerMat(matop.maxMat(matop.diag(B), 0), 0.5);

        // recover signs from off-diagonal
        axis[0][0] *= matop.sign((rotmat[2][1] - rotmat[1][2]));
        axis[1][0] *= matop.sign((rotmat[0][2] - rotmat[2][0]));
        axis[2][0] *= matop.sign((rotmat[1][0] - rotmat[0][1]));

    }
    else {
        std::vector<std::vector<double>> B = { {rotmat[2][1] - rotmat[1][2]}, {rotmat[0][2] - rotmat[2][0]}, {rotmat[1][0] - rotmat[0][1]} };
        axis = matop.factorMult(B, 1. / (2. * sin(angle)));
    }
    double n = sqrt(pow(axis[0][0], 2) + pow(axis[1][0], 2) + pow(axis[2][0], 2));

    std::vector<double> rv = { (axis[0][0] / n * angle), (axis[1][0] / n * angle), (axis[2][0] / n * angle) };
    for (unsigned int i = 0; i < rv.size(); i++) {
        rv[i] = abs(rv[i]);
    }
    // sign(rv[0]) == sign(rv[0]) => rotate clockwise
    // sign(rv[0]) != sign(rv[0]) => rotate counter-clockwise

    return rv;
    
}*/

std::vector<std::vector<double>> Orientation::compose(std::vector<std::vector<double>> A, std::vector<std::vector<double>> B) {
    std::vector<std::vector<double>> C(3, std::vector<double>(3, 0.0));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

std::vector<double> Orientation::rotMat2rv2(std::vector<std::vector<double>> rot){
    double theta = acos((rot[0][0]+rot[1][1]+rot[2][2]-1)/2);
    double theta_temp = 1/(2*sin(theta));
    std::vector<std::vector<double>> K = matop.factorMult({{rot[2][1]-rot[1][2]},{rot[0][2]-rot[2][0]},{rot[1][0]-rot[0][1]}},theta_temp);
    std::vector<double> KTranspose = {K[0][0], K[1][0], K[2][0]};
    theta_temp = theta / norm(KTranspose);
    K = matop.factorMult(K,theta_temp);
    KTranspose = {fabs(K[0][0]), fabs(K[1][0]), fabs(K[2][0])};
    return KTranspose;
}

std::vector<double> Orientation::getAngleAxisVector(double rotationZ, bool deg) { // takes input in degrees
    double rotRad;
    if (deg){
        rotRad = matop.degToRad(rotationZ);
    }
    else{
        rotRad = rotationZ;
    }
    std::vector<std::vector<double>> r1 = getRotMat({ 0,1,0 }, matop.PI);
    std::vector<std::vector<double>> r2 = getRotMat({ 0,0,1 }, rotRad);
    std::vector<std::vector<double>> r = compose(r2, r1);
    std::vector<double> rv = rotMat2rv2(r);
    // Determine clockwise/counter-clockwise rotation (NEEDS TESTING)
    if (rotationZ >= 0){
        rv[1] = -rv[1];
    }
    return rv;
}