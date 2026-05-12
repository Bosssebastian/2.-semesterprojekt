#pragma once
#ifdef PC_ENABLE_OPENCV
#include <opencv2/opencv.hpp>
#endif
#include <string>
#include <vector>

class VisionController
{
public:
    VisionController();
    ~VisionController();

    void scanForObject();

    bool objectReady();
    void getObjectPosition(std::vector<std::vector<double>> outputPos);
    void getObjectInfo(std::string&, std::string&, std::string&);

private:
#ifdef PC_ENABLE_OPENCV
    cv::VideoCapture* mCam;
    cv::QRCodeDetector mQR;
    cv::Mat mTempFrame, mFrame, mCorners;
#endif

    bool mStatus = false;    
    
    std::string mData;
    std::string mObject, mColor;
    int mSize = 0;
    
#ifdef PC_ENABLE_OPENCV
    cv::Point2d mPos;
#endif
    double mX, mY;

};
