#ifdef PC_ENABLE_OPENCV
#include <filesystem>
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

    // use a calibration already made
    const std::filesystem::path configPath = std::filesystem::path(__FILE__).parent_path() / "camera_params.yaml";
    cv::FileStorage fs(configPath.string(), cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr << "VisionController: failed to open calibration file " << configPath << "\n";
        delete mCam;
        mCam = nullptr;
        return;
    }
    // OpenCV automatically processes the rows, cols, and data tags
    fs["camera_matrix"] >> mCameraMatrix;
    fs["distortion_coefficients"] >> mDistCoeffs;
    fs.release();


    float S = 0.041 / 2.0; // Half-size of the QR code (meters)
    // the connors have to be giving the the way a cameras coordinate system is unlike a normal coordinate system.
    mObjectPoints.clear();
    mObjectPoints.push_back(cv::Point3f(-S, -S, 0));  // 0: Top-Left  (Negative X, Negative Y)
    mObjectPoints.push_back(cv::Point3f( S, -S, 0));  // 1: Top-Right (Positive X, Negative Y)
    mObjectPoints.push_back(cv::Point3f( S,  S, 0));  // 2: Bottom-Right (Positive X, Positive Y)
    mObjectPoints.push_back(cv::Point3f(-S,  S, 0));  // 3: Bottom-Left  (Negative X, Positive Y)

    // just for testing
    //mCam->retrieve(mTempFrame);
    //cv::imwrite("test.jpg",mTempFrame); 
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

    cv::Mat OldTransVec = (cv::Mat_<double>(3,1) <<  0, 0, 0);

    cv::Mat transDiff = mOldTransVec;
    while(true){
        if(!mStatus){
            sameSpot = 0;
            while(sameSpot < 5 || mData.empty()){
                // take an image and save it as mTempFrame
                mCam->grab();
                mCam->retrieve(mTempFrame);

                // checks if an image was taken if not then returns
                if (mTempFrame.empty()) {
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
                if (mCorners.empty() || mCorners.total() != 4) {
                    std::cerr << "VisionController: Not enough corners detected (" << mCorners.total() << ")\n";
                    sameSpot = 0;
                    continue;  // Continue the loop to try again
                }

                if (mCorners.rows != 4) {
                    mCorners = mCorners.reshape(2, 4);
                }

                try {
                    cv::solvePnP(mObjectPoints, mCorners, mCameraMatrix, mDistCoeffs, mRotVec, mTransVec);
                } catch (const cv::Exception& e) {
                    std::cerr << "VisionController: solvePnP failed: " << e.what() << "\n";
                    continue;
                }

                // just for testing
                // std::cout << mTransVec << "\n"; // print the location

                if (!mData.empty()) {
                    try{
                    getValue(mData, mObject, mColor, mSize);

                    // just for testing
                    //std::cout << "object: " << mObject << "\nsize: " << mSize << "\ncolor: " << mColor << "\n";
                    
                    }
                    catch (const cv::Exception& e) {
                    std::cerr << "VisionController: get value failed: " << e.what() << "\n";
                    mData.clear();
                    mCorners.release();
                    return;
                }
                }
                
                transDiff = OldTransVec - mTransVec;

                if (transDiff.at<double>(0,0) > -mMaxOff && transDiff.at<double>(0,0) < mMaxOff && transDiff.at<double>(1,0) > -mMaxOff && transDiff.at<double>(1,0) < mMaxOff && !mTransVec.empty() && !mData.empty()){
                    sameSpot++;
                }
                else{
                    sameSpot = 0;
                }
                OldTransVec = mTransVec;
            }
            std::cout << "An object was found\n";
            std::vector<std::vector<double>> outputPos;
            mStatus = true;
        }
    }
    
    return;
}

bool VisionController::objectReady(){
    return mStatus;
}

void VisionController::getObjectPosition(std::vector<std::vector<double>>& outputPos){
    //just in case the output vector is not intialzed correct
    if (outputPos.size() < 2) {
        outputPos.assign(2, std::vector<double>(1));
    }
    if (outputPos[0].size() < 1) {
        outputPos[0].resize(1);
    }
    if (outputPos[1].size() < 1) {
        outputPos[1].resize(1);
    }

    outputPos[0][0] = mTransVec.at<double>(0,0);
    outputPos[1][0] = mTransVec.at<double>(1,0);
}

void VisionController::getObjectInfo(std::string& object, std::string& size, std::string& color){
    object = mObject;
    size = mSize;
    color = mColor;
    return;
}

void VisionController::setStateFalse(){
    mStatus = false;
}

void VisionController::getObjectRot(double& output){
    output = mRotVec.at<double>(2,0); //fmod(mRotVec.at<double>(2,0), (M_PI/2.));
    std::cout << output << "\n";
    if (output < -M_PI/4.){
        output += M_PI/2.;
    }
    return;
}
#endif