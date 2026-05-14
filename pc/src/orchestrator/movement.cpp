#include "movement.h"

void Movement::move(std::vector<std::vector<double>> inputCoord, double speed, double acc, double customZ, double rotZ, bool isWorldFrame){
    std::vector<double> goal = transform.getMovementVec(inputCoord, rotZ, isWorldFrame);
    if (customZ != 0.0){
        goal[2] = customZ+0.12; // gripper = 12cm
        //goal[2] = 0.085; // stopper ved 0.5cm fra gripper til bord
        // 4 cm fra gripper til bord ved z=0
    }
    for (unsigned int i = 0; i < goal.size(); i++){
        //std::cout << goal[i] << "\n";
    }
    lastForwardPosition = goal; // save last linear position
    rtde_control.moveL(goal,speed,acc,asyncOn);
}

void Movement::moveJStock(std::string stock, double speed, double acc){
    int idx = 0; // Default to home position
    for (unsigned int i = 0; i < stockName.size(); i++){ // Determine stock position idx
        if (stock == stockName[i]){
            idx = i;
        }
    }
    std::vector<double> stockVec = stockCollection[idx];
    for (unsigned int i = 0; i < stockVec.size(); i++){ // Convert to radians
        stockVec[i] = matop.degToRad(stockVec[i]);
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

void Movement::moveZ(double diffZ, std::string state, double speed, double acc){
    //std::vector<double> curPos = rtde_control.getForwardKinematics(); // Does not work on RP (for some reason)
    //std::cout << "forward kinematics retrieved\n";
    std::vector<double> curPos;
    if (state == "base"){
        curPos = lastForwardPosition;
    }
    else if (state == "storage"){
        curPos = {-0.0842395, -0.340469, 0.337993, -1.93723, -2.473, 0.0};
    }
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
        distanceToCube = 0.196; // Measuring, Confirmed (14.5cm clearance), Needs test with mounted gripper
    }
    else if (start == "base") {
        distanceToCube = 0.215; // Measured, Confirmed (0.5cm clearance), Tested with mounted gripper
    }
    else {
        distanceToCube = 0; // Does not move on invallid starting position
    }
    moveZ(-distanceToCube, start, speed,acc);
}

void Movement::stop(){
    rtde_control.stopScript(); // stops script on robot
}