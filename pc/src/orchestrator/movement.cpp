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
        curPos = {-0.0842395, -0.340469, 0.337993, 0.754928, -3.04931, 0.0}; //-1.93723, -2.473, 0.0};
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


void Movement::moveUp(std::string start, double speed, double acc){ // Needs testing!!!
    std::vector<double> curPos = lastForwardPosition;
    curPos[1] = -0.45;
    rtde_control.moveL(curPos,speed,acc,asyncOn); // backup solution
    /*std::vector<double> curPos = lastForwardPosition;
    curPos[2] += 100;
    curPos = rtde_control.getInverseKinematics(curPos); // Convert to joint angles
    curPos.push_back(speed);
    curPos.push_back(acc);

    int idx = 0; // Default to home position
    for (size_t i = 0; i < stockName.size(); i++){ // Determine stock position idx
        if (start == stockName[i]){
            idx = i;
        }
    }
    std::vector<double> stockVec = stockCollection[idx];
    stockVec.push_back(speed);
    stockVec.push_back(acc);*/
    /*std::vector<std::vector<double>> path;
    if (start == "home"){
        std::cout << "path generating\n";
        std::vector<double> pose1 = stockCollection[0];
        for (size_t i = 0; i < pose1.size(); ++i){
            pose1[i] = matop.degToRad(pose1[i]);
            std::cout << pose1[i] << " ";
        }
        for (double v : pose1)
        {
            if (std::abs(v) > 6.3)
                std::cout << "BAD VALUE\n";
        }
        pose1.push_back(speed);
        pose1.push_back(acc);
        pose1.push_back(0.05);
        path.push_back(pose1);
        std::vector<double> pose2 = stockCollection[1];
        for (size_t i = 0; i < pose2.size(); ++i){
            pose2[i] = matop.degToRad(pose2[i]);
            std::cout << pose2[i] << " ";
        }
        for (double v : pose2)
        {
            if (std::abs(v) > 6.3)
                std::cout << "BAD VALUE\n";
        }
        pose2.push_back(speed);
        pose2.push_back(acc);
        pose2.push_back(0.0);
        path.push_back(pose2);
    }

    std::cout << "Path size: " << path.size() << std::endl;
    for (auto &p : path) {
        std::cout << "Waypoint size: " << p.size() << std::endl;
    }

    std::cout << "Connected: " << rtde_control.isConnected() << std::endl;
    bool ok = rtde_control.moveJ(path);
    std::cout << ok << " movement done\n";*/
    
}


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
    rtde_control.stopJ(1.0);
    rtde_control.stopL(1.0);
    rtde_control.stopScript(); // stops script on robot
}