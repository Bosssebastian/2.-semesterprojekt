#include <opencv2/opencv.hpp>
#include <vector>
#include <math.h>

class VisionController
{
public:
    VisionController();
    ~VisionController();

    void scanForObject();
    void setSameSpot(int);

    bool objectReady();
    void getObjectPosition(std::vector<std::vector<double>>& outputPos);
    void getObjectInfo(std::string&, std::string&, std::string&);
    void getObjectRot(double&);

private:
    cv::VideoCapture* mCam;
    cv::QRCodeDetector mQR;
    cv::Mat mTempFrame, mFrame, mCorners, mRotVec, mTransVec, mOldTransVec;
    std::vector<cv::Point3f> mObjectPoints;

    bool mStatus = false;    
    
    std::string mData;
    std::string mObject, mColor;
    int mSize;

    int sameSpot = 0;

    double mMaxOff = 0.005;

    double fx = 1920.0; 
    double fy = 1920.0;
    double cx = 1920.0 / 2.0;
    double cy = 1080.0 / 2.0;

    cv::Mat mCameraMatrix = (cv::Mat_<double>(3,3) << 
        fx,  0, cx,
        0, fy, cy,
        0,  0,  1);

    cv::Mat mDistCoeffs = cv::Mat::zeros(5, 1, CV_64F);

    int test = 0;

};
