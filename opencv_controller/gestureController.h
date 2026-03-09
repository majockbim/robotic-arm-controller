#ifndef GESTURE_CONTROLLER_H
#define GESTURE_CONTROLLER_H

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include "serial.h"
#include <vector>
#include <chrono>


const int SERVO_MIN = 0;
const int SERVO_MAX = 180;
const int SERVO_STEP = 5;  // how fast the angle moves


struct HandInfo {
    int fingerCount;
    cv::Point center;
    std::vector<cv::Point> fingerTips;
    cv::Rect boundingBox;
    double area;
};

class GestureController {
public:
    GestureController();
    ~GestureController();

    bool initCamera(int camIndex = 0);
    bool connectArduino(const std::string& portName);
    void run();
    void processFrame(cv::Mat& frame);
    HandInfo analyzeHand(const std::vector<cv::Point>& contour, cv::Mat& displayImg);
    void createTrackbars();
    std::chrono::steady_clock::time_point lastKeepaliveTime;
    bool lastSendFailed;

    // setters for real-time adjustment
    void setThreshold(int value) { threshValue = value; }
    void setBlurAmount(int value) { blurAmount = value; }
    void setDefectDepth(int value) { defectDepthThresh = value; }
    void setMinArea(int value) { minContourArea = value; }
    void setMaxArea(int value) { maxContourArea = value; }
    void setMorphSize(int value) { morphSize = value; }
    void setErosionIter(int value) { erosionIter = value; }
    void setDilationIter(int value) { dilationIter = value; }

private:
    // camera and serial communication
    cv::VideoCapture cap;
    SerialPort serial;

    // detection parameters
    int blurAmount;
    int threshValue;
    int minContourArea;
    int maxContourArea;
    int defectDepthThresh;
    int morphSize;
    int erosionIter;
    int dilationIter;

    // background learning
    cv::Mat backgroundRef;
    bool backgroundCaptured;
    int learningFrames;
    static const int LEARNING_FRAMES_NEEDED = 30;

    // gesture stability tracking
    int lastSentFingerCount;
    int stableFrames;
    static const int STABLE_FRAMES_REQUIRED = 10;

    // command timing and sending
    char lastSentCommand;
    int commandSendInterval;
    std::chrono::steady_clock::time_point lastCommandTime;

    // helper for sending repeated commands
    void sendCommandContinuously(char command);
};

#endif // GESTURE_CONTROLLER_H