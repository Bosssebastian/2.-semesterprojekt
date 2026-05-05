#include "movement.h"

void Movement::move(std::vector<std::vector<double>> inputCoord, double rotZ, double speed, double acc, bool isWorldFrame){
    std::vector<double> goal = transform.getMovementVec(inputCoord, rotZ, isWorldFrame);
    rtde_control.moveL(goal,speed,acc);
}

void Movement::home() {
    double base = 1.742013;
    double shoulder = -1.516168;
    double elbow = 1.857554;
    double wrist1 = -1.911310;
    double wrist2 = -1.570273;
    double wrist3 = 0.170344;
    rtde_control.moveJ({base,shoulder,elbow,wrist1,wrist2,wrist3},0.5,0.2);
}