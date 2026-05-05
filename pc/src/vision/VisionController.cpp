#include "VisionController.h"
#include <iostream>

/* just in case
std::string getJsonValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t startPos = json.find(searchKey);
    if (startPos == std::string::npos) return "";

    startPos += searchKey.length();
    
    // Skip optional whitespace and look for value
    size_t valStart = json.find_first_not_of(" \"", startPos);
    size_t valEnd = json.find_first_of("\",}", valStart);
    
    return json.substr(valStart, valEnd - valStart);
}
*/

void getValue(std::string& value, std::string& object, std::string& color, int& size)
/*input string, object type string, color string, size int*/
{
    object = value.substr(0,3);
    color = value.substr(4,3);
    size = stoi(value.substr(8,(value.length()-8)));
    return;
}







VisionController::VisionController()
    : mCam(nullptr)
    , mPos(0, 0)
    , mX(0)
    , mY(0)
{
    mCam = new cv::VideoCapture(0, cv::CAP_V4L2); // uses default camera on PC
    if (!mCam->isOpened()) {
        std::cerr << "VisionController: failed to open camera\n";
        delete mCam;
        mCam = nullptr;
        return;
    }


    // so the cam do not make shredded lines in the image
    mCam->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

    // set cam settings
    mCam->set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    mCam->set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    // turn off autofocus
    mCam->set(cv::CAP_PROP_AUTOFOCUS, 0);


    //
    //mCam->set(cv::CAP_PROP_SATURATION,)

    // give the camera time to start correctly
    for (int i = 0; i < 30; i++) {
        mCam->grab(); // Just grab the data, don't decode it yet to save time
    }

    //mCam->retrieve(mFrame);
    //cv::imwrite("fixed_capture.jpg", mFrame);
  
}

VisionController::~VisionController()
{
    delete mCam;
}

void VisionController::scanForObject()
{
    if (!mCam || !mCam->isOpened()) {
        std::cerr << "VisionController: camera unavailable\n";
        return;
    }

    // take an image and save it as mFrame
    mCam->grab();
    mCam->retrieve(mTempFrame);

     cv::cvtColor(mTempFrame, mFrame, cv::COLOR_BGR2GRAY);

    // checks if an image was taken if not then returns
    if (mFrame.empty()) {
        std::cerr << "VisionController: failed to capture image\n";
        return;
    }

    // detects the QR code in mFrame, then returns the decoded message to mData and the corners to mCorners
    try {
        mData = mQR.detectAndDecodeCurved(mFrame, mCorners);
    } catch (const cv::Exception& e) {
        std::cerr << "VisionController: QR decode failed: " << e.what() << "\n";
        mData.clear();
        mCorners.release();
        return;
    }

    mX = 0;
    mY = 0;
    int pointCount = 0;

    // find the center of the QR code
    if (!mCorners.empty() && mCorners.total() >= 4) {
        for (int row = 0; row < mCorners.rows; ++row) {
            for (int col = 0; col < mCorners.cols; ++col) {
                cv::Point2f pt = mCorners.at<cv::Point2f>(row, col);
                mX += pt.x;
                mY += pt.y;
                ++pointCount;
            }
        }

        if (pointCount == 4) {
            mX /= static_cast<double>(4);
            mY /= static_cast<double>(4);
            //std::cout << "x: " << mX << "\ny: " << mY << "\n";
        }
        spot_test++;
    }

    if (!mData.empty()) {

        //getValue(mData, mObject, mColor, mSize);

        /*
        mObject = getJsonValue(mData, "object");
        mSize = getJsonValue(mData, "size");
        mColor = getJsonValue(mData, "color");
        */
        mStatus = true;
        //std::cout << "object: " << mObject << "\nsize: " << mSize << "\ncolor: " << mColor << "\n";
        read_test++;
    }
}

bool VisionController::objectReady()
{
    return mStatus;
}

void VisionController::getObjectPosition(std::vector<std::vector<double>> outputPos)
{
    outputPos[0][0] = mX;
    outputPos[1][0] = mY;
    mStatus = false;
    return;
}


void VisionController::getObjectInfo(std::string& object, std::string& size, std::string& color)
{
    object = mObject;
    size = mSize;
    color = mColor;
    return;
}

