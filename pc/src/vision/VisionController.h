#include <opencv2/opencv.hpp>


class VisionController
{
public:
    VisionController();
    ~VisionController();

    void scanForObject();

    bool objectReady();
    void getObjectPosition(double outputPos[2][1]);

private:
    cv::VideoCapture mCam;
    cv::QRCodeDetector mQR;
    cv::Mat mFrame, mCorners, mRectImage;
    std::string mData;
    
    cv::Point2d mPos;

    double mX, mY;

};
