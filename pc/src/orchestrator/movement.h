#pragma once
#include "matrixoperations.h"
#include <string>
#include "transformations.h"
#include <ur_rtde/rtde_control_interface.h> // ur_rtde library to control ur_robot_arm

class Movement
{
public:
    Movement(std::string ip = "192.168.1.11", int port = 29998) : rtde_control(ur_rtde::RTDEControlInterface(ip,500,ur_rtde::RTDEControlInterface::FLAG_USE_EXT_UR_CAP,port)) {};
    void setTransform(double upZ = 17,bool isInwards = false, double rotZ = 22.5){transform = Transformations(double upZ,bool isInwards, double rotZ);}
    void move(std::vector<std::vector<double>>, double rotZ = -0.39269908169872415480783042290994, double speed = 0.5, double acc = 0.2, bool isWorldFrame = false); // world frame used for testing without cam
    void moveJStock(std::string);
    void home();
    void moveZ(double, double speed = 0.5, double acc = 0.2); // + goes up, - goes down
    ~Movement(){};
private:
    ur_rtde::RTDEControlInterface rtde_control;
    Transformations transform;
    MatrixOperations matop;
};