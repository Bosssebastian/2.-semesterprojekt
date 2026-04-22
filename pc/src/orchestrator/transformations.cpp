#include "transformations.h"

Transformations::Transformations(){}

Transformations::Transformations(double upZ,bool isInwards, double rotZ){
    baseToCamTransform = matop.findBaseToCamTrans(upZ, isInwards, rotZ);
    camToBaseTransform = matop.inverseMat(baseToCamTransform); // is not used (could be removed)
}

std::vector<std::vector<double>> Transformations::getBaseToCamTrans(){
    return baseToCamTransform;
}

std::vector<std::vector<double>> Transformations::getCamToBaseTrans(){
    return camToBaseTransform;
}

std::vector<double> Transformations::getMovementVec(std::vector<std::vector<double>> endpoint, bool testWithoutCam) {
	double posX,posY,posZ;
    if (endpoint.size() == 3){
        endpoint.push_back({1}); // add element for processing
        std::vector<std::vector<double>> transform;
        if (testWithoutCam) {
            transform = matop.getBaseToWorldTrans();
        }
        else {
            transform = baseToCamTransform;
        }
        std::vector<std::vector<double>> newPoint = matop.multMat(transform,endpoint);
        newPoint.pop_back(); // remove element
        posX = newPoint[0][0];
        posY = newPoint[1][0];
        posZ = newPoint[2][0];
    }
    else{ // move to home (example coordinates used for now)
        posX = -0.143;
        posY = -0.435;
        posZ = 0.20;
    }

	return {posX,posY,posZ, 0.00,3.14159,0.00}; // posX, posY, posZ are coordinates in base frame (in m). Last three values are tool orientation (in rad)
}