#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <math.h>

class VisionController
{
public:
    VisionController();
    ~VisionController();

    void scanForObject();

    bool objectReady();
    void getObjectPosition(std::vector<std::vector<double>>& outputPos);
    void getObjectInfo(std::string& object, std::string& size, std::string& color);
    void getObjectRot(double&);
    void setState(bool newState);

private:

    bool inBound();
    cv::VideoCapture* mCam;
    cv::QRCodeDetector mQR;
    cv::Mat mTempFrame, mFrame, mCorners, mRotVec, mTransVec, mOldTransVec;
    std::vector<cv::Point3f> mObjectPoints;

    bool mStatus = true;    
    
    std::string mData;
    std::string mObject, mColor, mSize;

    int sameSpot = 0;

    double mMaxOff = 0.0075;


    // vaules for camera calibartion
    cv::Mat mCameraMatrix = (cv::Mat_<double>(3,3))
    cv::Mat mDistCoeffs = cv::Mat::zeros(5, 1, CV_64F);

    int test = 0;

};