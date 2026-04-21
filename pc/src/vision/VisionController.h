#include <opencv2/opencv.hpp>
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
    cv::VideoCapture* mCam;
    cv::QRCodeDetector mQR;
    cv::Mat mTempFrame, mFrame, mCorners;

    bool mStatus = false;    
    
    std::string mData;
    std::string mObject, mColor;
    int mSize;
    
    cv::Point2d mPos;
    double mX, mY;

};
