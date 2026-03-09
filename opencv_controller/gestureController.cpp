#include "gestureController.h"
#include <iostream>
#include <chrono>

using namespace cv;
using namespace std;

#define SERVO_MIN 150
#define SERVO_MAX 600
#define SERVO_STEP 3    // step size, changable
static int servoPulse = (SERVO_MIN + SERVO_MAX) / 2;

int lastStableGesture = -1;
int gestureCounter = 0;
const int GESTURE_STABLE_FRAMES = 2;

// global pointer for trackbar callbacks
static GestureController* g_controller = nullptr;

// trackbar callback functions
void onThresholdChange(int value, void* userdata) {
    if (g_controller) {
        g_controller->setThreshold(value);
        cout << "Threshold: " << value << endl;
    }
}

void onBlurChange(int value, void* userdata) {
    if (g_controller) {
        g_controller->setBlurAmount(value);
        cout << "Blur: " << value << endl;
    }
}

void onMinAreaChange(int value, void* userdata) {
    if (g_controller) {
        g_controller->setMinArea(value * 100);
        cout << "Min Area: " << (value * 100) << endl;
    }
}

void onMaxAreaChange(int value, void* userdata) {
    if (g_controller) {
        g_controller->setMaxArea(value * 1000);
        cout << "Max Area: " << (value * 1000) << endl;
    }
}

void onDefectDepthChange(int value, void* userdata) {
    if (g_controller) {
        g_controller->setDefectDepth(value);
        cout << "Defect Depth: " << value << endl;
    }
}

void onMorphSizeChange(int value, void* userdata) {
    if (g_controller) {
        g_controller->setMorphSize(value);
        cout << "Morph Size: " << value << endl;
    }
}

void onErosionChange(int value, void* userdata) {
    if (g_controller) {
        g_controller->setErosionIter(value);
        cout << "Erosion Iterations: " << value << endl;
    }
}

void onDilationChange(int value, void* userdata) {
    if (g_controller) {
        g_controller->setDilationIter(value);
        cout << "Dilation Iterations: " << value << endl;
    }
}

GestureController::GestureController()
    : blurAmount(25),
      threshValue(60),
      minContourArea(5000),
      maxContourArea(100000),
      defectDepthThresh(30),
      morphSize(3),
      erosionIter(2),
      dilationIter(2),
      backgroundCaptured(false),
      learningFrames(0),
      lastSentFingerCount(-1),
      stableFrames(0),
      lastSentCommand('s'),  // 's' for stop - neutral command
      commandSendInterval(150)  // note: arduino timeout = 200ms
{
    g_controller = this;
    lastCommandTime = std::chrono::steady_clock::now();
    lastKeepaliveTime = std::chrono::steady_clock::now();
    lastSendFailed = false;
}

GestureController::~GestureController() {
    if (cap.isOpened()) {
        cap.release();
    }
    serial.disconnect();
    destroyAllWindows();
    g_controller = nullptr;
}

bool GestureController::initCamera(int camIndex) {
    if (cap.isOpened()) {
        cap.release();
    }

    if (!cap.open(camIndex, CAP_DSHOW)) {
        if (!cap.open(camIndex)) {
            cerr << "✗ Could not open camera " << camIndex << endl;
            return false;
        }
    }

    if (!cap.isOpened()) {
        cerr << "✗ Camera " << camIndex << " failed to open" << endl;
        return false;
    }

    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(CAP_PROP_FPS, 20);

    Mat testFrame;
    bool canRead = cap.read(testFrame);
    if (!canRead || testFrame.empty()) {
        cerr << "✗ Camera opened but cannot read frames" << endl;
        cap.release();
        return false;
    }

    cout << " Camera " << camIndex << " initialized successfully" << endl;
    cout << "  Resolution: " << testFrame.cols << "x" << testFrame.rows << endl;
    cout << "  FPS: " << cap.get(CAP_PROP_FPS) << endl;

    return true;
}

bool GestureController::connectArduino(const string& portName) {
    bool connected = serial.connect(portName, 9600);
    if (connected) {
        // give arduino time to reset after connection
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        cout << " Waiting for Arduino to initialize..." << endl;
    }
    return connected;
}

void GestureController::createTrackbars() {
    namedWindow("Controls", WINDOW_NORMAL);
    resizeWindow("Controls", 400, 400);

    createTrackbar("Threshold", "Controls", &threshValue, 255, onThresholdChange);
    createTrackbar("Blur", "Controls", &blurAmount, 51, onBlurChange);
    
    int minAreaScaled = minContourArea / 100;
    createTrackbar("Min Area (x100)", "Controls", &minAreaScaled, 200, onMinAreaChange);
    
    int maxAreaScaled = maxContourArea / 1000;
    createTrackbar("Max Area (x1000)", "Controls", &maxAreaScaled, 200, onMaxAreaChange);
    
    createTrackbar("Defect Depth", "Controls", &defectDepthThresh, 100, onDefectDepthChange);
    createTrackbar("Morph Size", "Controls", &morphSize, 10, onMorphSizeChange);
    createTrackbar("Erosion", "Controls", &erosionIter, 10, onErosionChange);
    createTrackbar("Dilation", "Controls", &dilationIter, 10, onDilationChange);

    cout << "\n Control trackbars created!" << endl;
    cout << "Adjust the sliders in the 'Controls' window to tune detection." << endl;
}


HandInfo GestureController::analyzeHand(const vector<Point>& contour, Mat& displayImg) {
    HandInfo hand;
    hand.fingerCount = 0;
    hand.area = contourArea(contour);

    vector<Point> approxContour;
    double epsilon = 0.0015 * arcLength(contour, true);
    approxPolyDP(contour, approxContour, epsilon, true);

    vector<int> hull;
    convexHull(approxContour, hull, false);

    if (hull.size() < 3) return hand;

    vector<Point> hullPoints;
    for (size_t k = 0; k < hull.size(); k++) {
        hullPoints.push_back(approxContour[hull[k]]);
    }

    hand.boundingBox = boundingRect(contour);
    Moments mu = moments(contour);
    if (mu.m00 != 0) {
        hand.center = Point(mu.m10 / mu.m00, mu.m11 / mu.m00);
    }

    float aspectRatio = (float)hand.boundingBox.width / (float)hand.boundingBox.height;

    vector<Vec4i> defects;
    convexityDefects(approxContour, hull, defects);

    vector<Point> fingerTips;
    int validDefects = 0;

    for (size_t k = 0; k < defects.size(); k++) {
        float depth = defects[k][3] / 256.0f;
        
        if (depth > defectDepthThresh) {
            int p_start = defects[k][0];
            int p_end = defects[k][1];
            int p_far = defects[k][2];

            Point ptStart = approxContour[p_start];
            Point ptEnd = approxContour[p_end];
            Point ptFar = approxContour[p_far];

            double a = norm(ptEnd - ptFar);
            double b = norm(ptStart - ptFar);
            double c = norm(ptStart - ptEnd);
            double angle = acos((b*b + c*c - a*a) / (2*b*c)) * 180.0 / CV_PI;

            if (angle < 90 && depth > defectDepthThresh) {
                validDefects++;
                fingerTips.push_back(ptStart);
                circle(displayImg, ptFar, 6, Scalar(0, 0, 255), -1);
                circle(displayImg, ptStart, 6, Scalar(0, 255, 0), -1);
            }
        }
    }

    double perimeter = arcLength(contour, true);
    double circularity = 4 * CV_PI * hand.area / (perimeter * perimeter);
    double convexArea = contourArea(hullPoints);
    double solidity = hand.area / convexArea;

    if (validDefects == 0) {
        if (circularity > 0.70 && solidity > 0.85) {
            hand.fingerCount = 0;
        } else if (aspectRatio < 0.6 || aspectRatio > 1.7) {
            hand.fingerCount = 1;
        } else if (solidity < 0.75) {
            hand.fingerCount = (aspectRatio > 1.0) ? 2 : 1;
        } else {
            hand.fingerCount = 1;
        }
    } else if (validDefects == 1) {
        hand.fingerCount = (solidity < 0.65 || aspectRatio > 1.4) ? 2 : 1;
    } else if (validDefects == 2) {
        hand.fingerCount = 3;
    } else if (validDefects == 3) {
        hand.fingerCount = 4;
    } else if (validDefects >= 4) {
        hand.fingerCount = 5;
    }

    if (hand.fingerCount > 5) hand.fingerCount = 5;

    hand.fingerTips = fingerTips;

    circle(displayImg, hand.center, 8, Scalar(255, 0, 255), -1);
    
    vector<vector<Point>> hullVec = {hullPoints};
    drawContours(displayImg, hullVec, 0, Scalar(0, 255, 255), 2);
    rectangle(displayImg, hand.boundingBox, Scalar(255, 0, 0), 2);

    String debugText = "S:" + to_string((int)(solidity * 100)) + "% C:" + to_string((int)(circularity * 100)) + "%";
    putText(displayImg, debugText, Point(hand.boundingBox.x, hand.boundingBox.y - 10), 
            FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 0), 1);

    return hand;
}

void GestureController::processFrame(Mat& frame) {
    if (frame.empty()) {
        cerr << "Warning: Empty frame received" << endl;
        return;
    }

    Mat flipped, gray, blurred, diff, thresh;
    flip(frame, flipped, 1);

    // define ROI and draw box
    Rect roi(80, 60, 480, 360);
    rectangle(flipped, roi, Scalar(0, 255, 0), 2);
    Mat roiImg = flipped(roi).clone();

    // imave pre-processing
    cvtColor(roiImg, gray, COLOR_BGR2GRAY);

    // learning background phase
    if (!backgroundCaptured) {
        if (learningFrames == 0) {
            gray.convertTo(backgroundRef, CV_32F);
        }

        Mat grayFloat;
        gray.convertTo(grayFloat, CV_32F);
        // slowly update the background average
        accumulateWeighted(grayFloat, backgroundRef, 0.1); 
        learningFrames++;

        String learningText = "Learning background... " + to_string(learningFrames) + "/" + to_string(LEARNING_FRAMES_NEEDED);
        putText(flipped, learningText, Point(20, 40), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255), 2);
        putText(flipped, "Keep hands out of green box!", Point(20, 70), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255), 2);

        if (learningFrames >= LEARNING_FRAMES_NEEDED) {
            backgroundCaptured = true;
            Mat temp;
            backgroundRef.convertTo(temp, CV_8U);
            backgroundRef = temp.clone();
            cout << "✓ Background learning complete!" << endl;
        }

        imshow("Gesture Control", flipped);
        return;
    }
    // background learning phase end

    // background subracting, threshold
    absdiff(gray, backgroundRef, diff);

    int blur = blurAmount;
    if (blur % 2 == 0) blur++;
    if (blur < 1) blur = 1;

    GaussianBlur(diff, blurred, Size(blur, blur), 0);
    threshold(blurred, thresh, threshValue, 255, THRESH_BINARY);

    // erode/dilate
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(morphSize * 2 + 1, morphSize * 2 + 1));
    erode(thresh, thresh, kernel, Point(-1, -1), erosionIter);
    dilate(thresh, thresh, kernel, Point(-1, -1), dilationIter);

    // contour finding
    vector<vector<Point>> contours;
    findContours(thresh.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<pair<double, int>> validContours;
    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        if (area > minContourArea && area < maxContourArea) {
            validContours.push_back(make_pair(area, i));
        }
    }

    sort(validContours.begin(), validContours.end(), greater<pair<double, int>>());

    vector<HandInfo> hands;
    Mat displayRoi = roiImg.clone();
    int fingerCount = -1;

    // process largest contour (max 1 hand)
    if (validContours.size() > 0) {
        int idx = validContours[0].second;
        HandInfo hand = analyzeHand(contours[idx], displayRoi);
        hands.push_back(hand);
        fingerCount = hand.fingerCount;

        Point textPos(hand.boundingBox.x + 10, hand.boundingBox.y + 50);
        putText(displayRoi, to_string(hand.fingerCount), textPos, 
                FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255), 4);
    }

    //  servo control logic start

    // map gesture to action ID: 1 -> incr pulse, 2 -> decr pulse, 0 -> stop/hold
    int detectedGesture = 0; // 0 = HOLD (default behavior if no specific gesture is found)
    if (fingerCount == 1) {
        detectedGesture = 1;      // 1 finger: incr pulse
    } else if (fingerCount == 5) {
        detectedGesture = 2;      // 5 fingers: decr pulse
    }

    // command variables for status display
    std::string statusMessage = "HOLD/STOP"; // Will be updated below
    Scalar statusColor = Scalar(100, 100, 100);

    // only proceed if the gesture has been stable
    if (detectedGesture == lastStableGesture) {
        gestureCounter++;
    } else {
        gestureCounter = 0;
        lastStableGesture = detectedGesture;
    }

    if (gestureCounter >= GESTURE_STABLE_FRAMES) {
        // rate limit check
        static auto lastMove = std::chrono::steady_clock::now();
        auto now2 = std::chrono::steady_clock::now();
        long dt = std::chrono::duration_cast<std::chrono::milliseconds>(now2 - lastMove).count();

        // rate limiter
        const int SEND_INTERVAL_MS = 25; 
        
        // decide if we should move/send a command
        if (dt >= SEND_INTERVAL_MS) {

            bool pulseChanged = false;

            if (detectedGesture == 1) {
                servoPulse += SERVO_STEP;
                pulseChanged = true;
                statusMessage = "1 FINGER: MOVE LEFT (+)";
                statusColor = Scalar(0, 255, 0);
            }
            else if (detectedGesture == 2) {
                servoPulse -= SERVO_STEP;
                pulseChanged = true;
                statusMessage = "5 FINGERS: MOVE RIGHT (-)";
                statusColor = Scalar(0, 0, 255);
            }
            else {
                statusMessage = "HOLD/STOP (Other)";
            }

            servoPulse = std::clamp(servoPulse, SERVO_MIN, SERVO_MAX);

            if (pulseChanged && serial.isConnected()) {
                serial.sendString("A" + std::to_string(servoPulse) + "\n");
            }

            lastMove = now2;
        }

    }
    
    
    
    // combine status and pulse for main display
    std::string currentStatus = statusMessage + " | Pulse: " + std::to_string(servoPulse);
    
    // draw black background for readability
    int baseline = 0;
    Size textSize = getTextSize(currentStatus, FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseline);
    rectangle(flipped, Point(15, 15), Point(25 + textSize.width, 55), Scalar(0, 0, 0), -1);
    
    // draw main status text
    putText(flipped, currentStatus, Point(20, 45), FONT_HERSHEY_SIMPLEX, 0.8, statusColor, 2);

    // draw finger count
    if (hands.size() > 0) {
        String fingerText = "Fingers: " + to_string(hands[0].fingerCount);
        putText(flipped, fingerText, Point(20, 80), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 2);
    }
    
    // draw parameters
    String paramText = "T:" + to_string(threshValue) + " B:" + to_string(blurAmount) + " D:" + to_string(defectDepthThresh);
    putText(flipped, paramText, Point(20, 120), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 200, 0), 1);

    // draw controls
    putText(flipped, "Press 'R' to reset | ESC to exit", Point(20, flipped.rows - 20), 
            FONT_HERSHEY_SIMPLEX, 0.6, Scalar(200, 200, 200), 1);

    // copy processed ROI back to flipped frame
    displayRoi.copyTo(flipped(roi));

    // show final windows
    imshow("Gesture Control", flipped);
    imshow("Threshold", thresh);
}

void GestureController::run() {
    if (!cap.isOpened()) {
        cerr << "✗ Camera not initialized!" << endl;
        return;
    }

    cout << "\n=== Gesture Controller Started ===" << endl;
    cout << "Instructions:" << endl;
    cout << "1. Keep hands OUT of the green box for 1-2 seconds" << endl;
    cout << "2. Program will learn the background" << endl;
    cout << "3. Then place hand in the box to detect" << endl;
    cout << "\nGestures:" << endl;
    cout << " NO HAND = Switch servo every 2 seconds" << endl;
    cout << " 1 finger = Move BACKWARD continuously" << endl;
    cout << " 5 fingers (open palm) = Move FORWARD continuously" << endl;
    cout << " Other finger counts = STOP movement" << endl;
    cout << "\nControls:" << endl;
    cout << " R = Reset background" << endl;
    cout << " ESC = Exit" << endl;
    cout << "\nTuning:" << endl;
    cout << " Use trackbars in 'Controls' window to adjust detection" << endl;
    cout << "===================================\n" << endl;

    createTrackbars();

    Mat frame;
    int frameCount = 0;
    int failedFrames = 0;

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    cout << "✓ System stabilized — starting frame processing..." << endl;
    while (true) {
        bool success = cap.read(frame);
        
        if (!success || frame.empty()) {
            failedFrames++;
            cerr << "Warning: Failed to read frame (attempt " << failedFrames << ")" << endl;
            
            if (failedFrames > 30) {
                cerr << "✗ Too many failed frames. Stopping..." << endl;
                break;
            }
            continue;
        }

        failedFrames = 0;
        frameCount++;

        processFrame(frame);

        char key = waitKey(30);
        
        if (key == 27) { // ESC
            cout << "ESC pressed. Exiting..." << endl;
            break;
        }
        
        if (key == 'r' || key == 'R') {
            backgroundCaptured = false;
            learningFrames = 0;
            lastSentCommand = 's';
            cout << "Resetting background... Keep hands out!" << endl;
        }
    }

    cout << "Processed " << frameCount << " frames" << endl;
    destroyAllWindows();
}