#pragma once
#include "matrixoperations.h"
#include <string>
#include "transformations.h"
#include <ur_rtde/rtde_control_interface.h> // ur_rtde library to control ur_robot_arm

class Movement
{
public:
    Movement(std::string ip = "192.168.1.11", int port = 29998) : rtde_control(ur_rtde::RTDEControlInterface(ip,500,ur_rtde::RTDEControlInterface::FLAG_USE_EXT_UR_CAP,port)) {};
    void setTransform(double upZ = 14,bool isInwards = false, double rotZ = 22.5){transform = Transformations(upZ, isInwards, rotZ);}
    void move(std::vector<std::vector<double>>, double speed = 0.5, double acc = 0.2, double customZ = 0.0, double rotZ = 0.39269908169872415480783042290994, bool isWorldFrame = false); // world frame used for testing without cam
    void moveJStock(std::string, double speed = 0.5, double acc = 0.2);
    void home(); // Shortcut for moveJStock("home")
    void moveZ(double, std::string, double speed = 0.5, double acc = 0.2); // + goes up, - goes down
    void moveUp(std::string, double speed = 0.5, double acc = 0.2); // Could not get async to work with path planning. Timing issue, which requires us to stall the program while it gets ready
    void moveDown(std::string, double speed = 0.5, double acc = 0.2);
    bool isDone();
    void toggleAsync();
    void stop();
    ~Movement(){};
private:
    ur_rtde::RTDEControlInterface rtde_control;
    Transformations transform;
    MatrixOperations matop;
    std::vector<double> stockHome = {99.81, -86.87, 106.43, -109.51, -90, 9.76}; // in degrees
    std::vector<double> stockStorage = {57.90, -110.39, 120.28, -99.85, -90, -60.0}; // in degrees
    std::vector<std::vector<double>> stockCollection = {stockHome,stockStorage};
    std::vector<std::string> stockName = {"home","storage"};
    std::vector<double> lastForwardPosition;
    bool asyncOn = true;
};