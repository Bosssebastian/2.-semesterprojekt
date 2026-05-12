#include "movement.h"

void Movement::move(std::vector<std::vector<double>> inputCoord, double rotZ, double speed, double acc, double customZ, bool isWorldFrame){
    std::vector<double> goal = transform.getMovementVec(inputCoord, rotZ, isWorldFrame);
    if (customZ != 0.0){
        goal[2] = customZ;
    }
    rtde_control.moveL(goal,speed,acc,asyncOn);
}

void Movement::moveJStock(std::string stock, double speed, double acc){
    int idx = 0; // Default to home position
    for (int i = 0; i < stockName.size(); i++){ // Determine stock position idx
        if (stock == stockName[i]){
            idx = i;
        }
    }
    std::vector<double> stockVec = stockCollection[idx];
    for (int i = 0; i < stockVec.size(); i++){ // Convert to radians
        stockVec[i] = matop.degToRad(stockVec[i])
    }
    rtde_control.moveJ(stockVec,speed,acc,asyncOn); // Move to pose
}

void Movement::home() {
    double base = 1.742013;
    double shoulder = -1.516168;
    double elbow = 1.857554;
    double wrist1 = -1.911310;
    double wrist2 = -matop.PI_2;
    double wrist3 = 0.170344;
    rtde_control.moveJ({base,shoulder,elbow,wrist1,wrist2,wrist3},0.5,0.2,asyncOn);
}

void Movement::moveZ(double diffZ, double speed, double acc){
    std::vector<double> curPos = rtde_control.getForwardKinematics();
    curPos[2] += diffZ;
    rtde_control.moveL(curPos,speed,acc,asyncOn);
}

bool Movement::isDone(){
    if (rtde_control.getAsyncOperationProgress() >= 0){
        return false;
    }
    else {
        return true;
    }
}

void Movement::toggleAsync(){
    if (asyncOn){
        asyncOn = false;
    }
    else {
        asyncOn = true;
    }
}

/*
void Movement::moveUp(std::string start, double speed, double acc){ // Needs testing!!!
    std::vector<double> curPos = rtde_control.getForwardKinematics();
    curPos[2] += 100;
    curPos = rtde_control.getInverseKinematics(curPos); // Convert to joint angles
    curPos.push_back(speed);
    curPos.push_back(acc);

    int idx = 0; // Default to home position
    for (int i = 0; i < stockName.size(); i++){ // Determine stock position idx
        if (stock == stockName[i]){
            idx = i;
        }
    }
    std::vector<double> stockVec = stockCollection[idx];
    stockVec.push_back(speed);
    stockVec.push_back(acc);
    rtde_control.moveJ({curPos,stockVec},asyncOn); // must have at least 3 points and ended by the stopScript() function
}
*/

void Movement::moveDown(std::string start, double speed, double acc){
    double distanceToCube;
    if (start == "storage"){
        distanceToCube = 0.173; // Measuring, Confirmed (14.5cm clearance), Needs test with mounted gripper
    }
    else if (start == "home") {
        distanceToCube = 0.19; // Measured, Confirmed (14.5cm clearance), Needs test with mounted gripper
    }
    else {
        distanceToCube = 0; // Does not move on invallid starting position
    }
    moveZ(-distanceToCube,speed,acc);
}