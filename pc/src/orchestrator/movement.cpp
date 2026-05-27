#include "movement.h"

void Movement::move(std::vector<std::vector<double>> inputCoord, double speed, double acc, double customZ, double rotZ, bool isWorldFrame){
    std::vector<double> goal = transform.getMovementVec(inputCoord, rotZ, isWorldFrame);
    if (customZ != 0.0){
        goal[2] = customZ+0.12; // gripper = 12cm
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

void Movement::home(double speed, double acc) {
    double base = 1.742013;
    double shoulder = -1.516168;
    double elbow = 1.857554;
    double wrist1 = -1.911310;
    double wrist2 = -matop.PI_2;
    double wrist3 = 0.170344;
    rtde_control.moveJ({base,shoulder,elbow,wrist1,wrist2,wrist3},speed,acc,asyncOn);
}

void Movement::moveZ(double diffZ, std::string state, double speed, double acc){
    std::vector<double> curPos;
    if (state == "base"){
        curPos = lastForwardPosition;
    }
    else if (state == "storage"){
        curPos = {-0.0842395, -0.340469, 0.337993, 0.754928, -3.04931, 0.0};
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


void Movement::moveUp(std::string start, double speed, double acc){
    std::vector<double> curPos = lastForwardPosition;
    if (start == "base"){
        curPos[1] = -0.45;
        curPos[0] = 0.15;
        rtde_control.moveL(curPos,speed,acc,asyncOn);
    }
    else if (start == "output"){
        rtde_control.moveL(curPos,speed,acc,asyncOn);
    }
}


void Movement::moveDown(std::string start, double speed, double acc){
    double distanceToCube;
    if (start == "storage"){
        distanceToCube = 0.196; // Measuring, Confirmed (0.5cm clearance), Tested with mounted gripper
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
    rtde_control.stopJ(1.0);
    rtde_control.stopL(1.0);
    rtde_control.stopScript(); // stops script on robot
}

void Movement::testScript(int maxIterations, double speed, double acc, int version){
    if (version == 1){
        Transformations testTransform(14.*5.);
        std::vector<double> point11 = testTransform.getMovementVec({{(0.05*3.)},{-(0.05*10.)},{0.25}},matop.degToRad(22.5),true);
        std::vector<double> point21 = testTransform.getMovementVec({{(0.05*4.)},{-(0.05*14.)},{0.25}},matop.degToRad(22.5),true);
        std::vector<double> point12 = point11;
        point12[2] = 0.12;
        std::vector<double> point22 = point21;
        point22[2] = 0.12;
        std::vector<std::vector<double>> waypoints = {point11, point12, point21, point22};
        for (size_t i = 0; i < (waypoints.size() * maxIterations); ++i){
            rtde_control.moveL(waypoints[(i % waypoints.size())],speed,acc);
        }
    }
    else if (version == 2){
        std::vector<double> point1 = transform.getMovementVec({{(0.05*3.)},{-(0.05*10.)},{0.1}},matop.degToRad(22.5-90),true);
        std::vector<double> point2 = transform.getMovementVec({{(0.05*3.)},{-(0.05*17.)},{0.1}},matop.degToRad(22.5-90),true);
        std::vector<double> point3 = transform.getMovementVec({{-(0.05*3.)},{-(0.05*16.)},{0.1}},matop.degToRad(22.5),true);
        std::vector<double> point4 = transform.getMovementVec({{-(0.05*1.)},{-(0.05*12.)},{0.1}},matop.degToRad(22.5),true);
        std::vector<std::vector<double>> waypoints = {point1, point2, point3, point4};
        rtde_control.moveL(waypoints[0],speed,acc);
        rtde_control.moveL(waypoints[1],speed,acc);
        rtde_control.moveL(waypoints[2],speed,acc);
        rtde_control.moveL(waypoints[3],speed,acc);

        rtde_control.moveL(waypoints[1],speed+0.2,acc+0.2);
        rtde_control.moveL(waypoints[2],speed-0.1,acc);

        rtde_control.moveL(waypoints[0],speed,acc);
        rtde_control.moveL(waypoints[3],speed,acc);
    }
}

void Movement::toOutput(double speed, double acc, bool test){
    std::vector<double> outputZone;
    if (test){
        outputZone = transform.getMovementVec({{-0.05*2+0.005},{-0.05*15-0.005},{0.325}},matop.degToRad(22.5),true);
        lastForwardPosition = outputZone;
        outputZone[2] = 0.09;
    }
    else{
        outputZone = transform.getMovementVec({{0.05*5+0.005},{-0.05*5-0.005},{0.325}},matop.degToRad(22.5-90),true);
        lastForwardPosition = outputZone;
        outputZone[2] = 0.1;
    }
    rtde_control.moveL(outputZone,speed,acc,asyncOn);;
}