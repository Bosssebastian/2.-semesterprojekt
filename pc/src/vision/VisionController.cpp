#include "VisionController.h"

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


VisionController::VisionController()
{
    mCam = cv::VideoCapture(0, cv::CAP_V4L2); // uses defalut camare on pc
    
    // so the cam do not make shredded lines in the image
    mCam.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

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
    // take an image and save it as mFrame
    mCam.grab();
    mCam.retrieve(mFrame);

    // checks if an image was taken if not then returns
    if (mFrame.empty()) {
        std::cout << "failed to capture image\n";
        return;
    }

    // detects the QR code in mFrame, then returns the decoded message to mData and the corners to mCorners
    mData = mQR.detectAndDecodeCurved(mFrame, mCorners);
    std::cout << mData << "\n";

    // find the center of the QR code
    if (!mCorners.empty()) {
        int corners = mCorners.cols; // Usually 4 corners

        for (int i = 0; i < corners; i++) {
            // pooints is typically a 1x4 CV_32FC2 matrix or 4x2 CV_32F
            cv::Point2f pt = mCorners.at<cv::Point2f>(i);
            mX += pt.x;
            mY += pt.y;
        }
        mX /= 4.;
        mY /= 4.;
        std::cout << "x: " << mX <<"\ny: " << mY << "\n";
    }
    if (!mData.empty())
    {
        mObject = getJsonValue(mData, "object");
        mSize = getJsonValue(mData, "size");
        mColor = getJsonValue(mData, "color");
        std::cout << "object: " << mObject << "\nsize: " << mSize << "\ncolor: " << mColor << "\n";
    }
}

bool VisionController::objectReady()
{
    return false;
}

void VisionController::getObjectPosition(double outputPos[2][1])
{
    outputPos[0][0] = mX;
    outputPos[1][0] = mY;
    return;
}

