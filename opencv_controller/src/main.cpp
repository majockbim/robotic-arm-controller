#include "gestureController.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    cout << "=== OpenCV Gesture Controller ===" << endl;
    cout << "Initializing..." << endl;
    
    GestureController controller;
    
    // Try multiple camera indices
    bool cameraFound = false;
    int camIndex = 0;
    
    cout << "\nSearching for available cameras..." << endl;
    for (int i = 0; i < 3; i++) {
        cout << "Trying camera index " << i << "..." << endl;
        if (controller.initCamera(i)) {
            cameraFound = true;
            camIndex = i;
            cout << "✓ Camera found at index " << i << endl;
            break;
        }
        // Small delay between attempts
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    
    if (!cameraFound) {
        cerr << "\n✗ Failed to initialize any camera!" << endl;
        cerr << "\nTroubleshooting steps:" << endl;
        cerr << "1. Check if camera is connected" << endl;
        cerr << "2. Close any other applications using the camera" << endl;
        cerr << "3. Check camera permissions in Windows Settings" << endl;
        cerr << "4. Try running as Administrator" << endl;
        cerr << "\nPress Enter to exit...";
        cin.get();
        return -1;
    }
    
    // Connect to Arduino
    string portName;
    cout << "\nEnter COM port (e.g., COM3): ";
    cin >> portName;
    
    if (!controller.connectArduino(portName)) {
        cerr << "\n✗ Failed to connect to Arduino!" << endl;
        cerr << "Check Device Manager for the correct port." << endl;
        cerr << "\nContinuing without Arduino connection..." << endl;
        cerr << "You can still test gesture detection." << endl;
        this_thread::sleep_for(chrono::seconds(2));
    } else {
        cout << "✓ Arduino connected successfully" << endl;
    }
    
    // Run gesture control
    cout << "\nStarting gesture controller..." << endl;
    this_thread::sleep_for(chrono::milliseconds(500));
    controller.run();
    
    cout << "\nGesture controller stopped." << endl;
    return 0;
}