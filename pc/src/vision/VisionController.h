#include <opencv2/opencv.hpp>


class VisionController
{
public:
    VisionController();
    ~VisionController();

    void scanForObject();

    bool objectReady();
    void getObjectPosition(double outputPos[2]);

private:
    cv::VideoCapture mCam;
    cv::Mat mFrame, mCorners, mRectImage, mPos;

};
