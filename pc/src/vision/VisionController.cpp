#include "VisionController.h"

VisionController::VisionController()
{
    mCam = cv::VideoCapture(0, cv::CAP_V4L2); // uses defalut camare on pc
    
    // set cam settings
    mCam.set(3,1920); // width
    mCam.set(4,1080); // height

    // give the camare time to start correctly
    for(int i = 0; i < 30; i++) 
    {
        mCam.grab(); // Just grab the data, don't decode it yet to save time
    }

}

VisionController::~VisionController()
{
    // Deconstructor
}

void VisionController::scanForObject()
{

}

bool VisionController::objectReady()
{

}

void VisionController::getObjectPosition(double outputPos[2])
{
    outputPos[0] = mPos.x;
    outputPos[1] = mPos.y;
    return;
}