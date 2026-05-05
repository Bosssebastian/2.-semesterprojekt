#include "movement.h"

void Movement::move(std::vector<std::vector<double>> inputCoord, double rotZ, double speed, double acc, bool isWorldFrame){
    std::vector<double> goal = transform.getMovementVec(inputCoord, rotZ, isWorldFrame);
    rtde_control.moveL(goal,speed,acc);
}

void Movement::moveJStock(std::string stock){
    std::vector<double> stockHome = {99.81, -86.87, 106.43, -109.51, -90, 9.76};
    std::vector<double> stockStorage = {62.33, -107.91, 114.55, -96.59, -90, 40.34};
    std::vector<std::vector<double>> stockCollection = {stockHome,stockStorage};
    std::vector<std::string> stockName = {"home","storage"};
    int idx = 0; // Default to home position
    for (int i = 0; i < stockName.size(); i++){ // Determine stock position idx
        if (stock == stockName[i]){
            idx = i;
        }
    }
    for (int i = 0; i < stockCollection[idx].size(); i++){ // Convert to radians
        stockCollection[idx][i] = matop.degToRad(stockCollection[idx][i])
    }
    rtde_control.moveJ(stockCollection[idx],0.5,0.2); // Move to pose
}

void Movement::home() {
    double base = 1.742013;
    double shoulder = -1.516168;
    double elbow = 1.857554;
    double wrist1 = -1.911310;
    double wrist2 = -matop.PI_2;
    double wrist3 = 0.170344;
    rtde_control.moveJ({base,shoulder,elbow,wrist1,wrist2,wrist3},0.5,0.2);
}

void Movement::moveZ(double diffZ, double speed, double acc){
    std::vector<double> curPos = rtde_control.getForwardKinematics();
    curPos[2] += diffZ;
    rtde_control.moveL(curPos,speed,acc);
}