#include "VisionController.h"
#include <iostream>

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

    // give the camera time to start correctly
    for (int i = 0; i < 30; i++) {
        mCam->grab(); // Just grab the data, don't decode it yet to save time
    }

    // use a calibriation already made
    cv::FileStorage fs("camera_params.yaml", cv::FileStorage::READ);
    fs["camera_matrix"] >> mCameraMatrix;
    fs["distortion_coefficients"] >> mDistCoeffs;
    fs.release();

    float S = 0.045; // size of the qr Code
    mObjectPoints.push_back(cv::Point3f(0, 0, 0));       // Top-Left
    mObjectPoints.push_back(cv::Point3f(S, 0, 0));       // Top-Right
    mObjectPoints.push_back(cv::Point3f(S, S, 0));       // Bottom-Right
    mObjectPoints.push_back(cv::Point3f(0, S, 0));       // Bottom-Left

    /* 
    // save the first image
    mCam->retrieve(mFrame);
    cv::imwrite("fixed_capture.jpg", mFrame);
    */
}

VisionController::~VisionController()
{
    delete mCam;
}

void VisionController::scanForObject(){
    // checks if the camera is open
    if (!mCam || !mCam->isOpened()) {
        std::cerr << "VisionController: camera unavailable\n";
        return;
    }

    int sameSpot = 0;

    cv::Mat OldTransVec = (cv::Mat_<double>(3,1) <<  0, 0, 0);

    cv::Mat transDiff = mOldTransVec;

    while(sameSpot < 5){
        // take an image and save it as mTempFrame
        mCam->grab();
        mCam->retrieve(mTempFrame);

        // checks if an image was taken if not then returns
        if (mFrame.empty()) {
            std::cerr << "VisionController: failed to capture image\n";
            return;
        }

        // then convert the image into grayscale
        cv::cvtColor(mTempFrame, mFrame, cv::COLOR_BGR2GRAY);

        // detects the QR code in mFrame, then returns the decoded message to mData and the corners to mCorners
        try {
            mData = mQR.detectAndDecodeCurved(mFrame, mCorners);
        } catch (const cv::Exception& e) {
            std::cerr << "VisionController: QR decode failed: " << e.what() << "\n";
            mData.clear();
            mCorners.release();
            return;
        }

        // Check if we have exactly 4 corners for solvePnP
        if (mCorners.rows != 4) {
            std::cerr << "VisionController: Not enough corners detected (" << mCorners.rows << ")\n";
            continue;  // Continue the loop to try again
        }

        cv::solvePnP(mObjectPoints, mCorners, mCameraMatrix, mDistCoeffs, mRotVec, mTransVec);

        std::cout << mTransVec << "\n";

        if (!mData.empty()) {
            getValue(mData, mObject, mColor, mSize);

            mStatus = true;
            std::cout << "object: " << mObject << "\nsize: " << mSize << "\ncolor: " << mColor << "\n";
        }
        
        transDiff = OldTransVec - mTransVec;

        if (transDiff.at<double>(0,0) > -mMaxOff && transDiff.at<double>(0,0) < mMaxOff && transDiff.at<double>(1,0) > -mMaxOff && transDiff.at<double>(1,0) < mMaxOff){
            sameSpot++;
        }
        else{
            sameSpot = 0;
        }
        OldTransVec = mTransVec;
    }
    std::cout << "a object was found\n";
}

bool VisionController::objectReady(){
    return mStatus;
}

void VisionController::getObjectPosition(std::vector<std::vector<double>> outputPos){
    outputPos[0][0] = mTransVec.at<double>(0,0);
    outputPos[1][0] = mTransVec.at<double>(1,0);
    mStatus = false;
    return;
}


void VisionController::getObjectInfo(std::string& object, std::string& size, std::string& color){
    object = mObject;
    size = mSize;
    color = mColor;
    return;
}

