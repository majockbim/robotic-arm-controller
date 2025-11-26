# Robotic Arm Controller

Control a 3D-printed robotic arm using **two different input methods**:

- **Hand Gestures Controller (OpenCV)**
- **Keyboard Controller (Serial Monitor)**

This project applies embedded systems and computer vision to control a physical robotic arm in real time.
---

# Project Structure
```
robotic-arm-controller
├── include
│   └── keyboardController.h
│
├── opencv_controller
│   ├── CMakeLists.txt
│   ├── gestureController.cpp
│   ├── gestureController.h
│   ├── serial.cpp
│   ├── serial.h
│   ├── .vscode
│   └── src
│       └── main.cpp
│
├── platformio.ini
│
└── src
    ├── output
    ├── keyboardController.cpp
    └── main.cpp
```
---

# How to Run

## 1. Flash the Arduino (Main Controller)

**Build / Compile**
Using PlatformIO:
```
pio run
```
Upload to Arduino
```
pio run --target upload
```
Open Serial Monitor
```
pio device monitor
```

You will be asked to pick a mode:
- Keyboard Controller
- Gesture Controller

## Keyboard Controller Mode

If you select **Keyboard Controller**:
- Stay in the PlatformIO Serial Monitor
- Press keys to move the robotic arm joints
- Commands are sent directly to the Arduino through serial

## Gesture Controller Mode (OpenCV)

If you select **Gesture Controller**, then:

1. Close the PlatformIO serial monitor
(the OpenCV program needs the serial port)
2. Build the OpenCV gesture controller:
```
cd opencv_controller
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

3. Run the gesture controller executable:
```
.\Release\opencv_controller.exe
```

This will:

- Open your webcam
- Detect hand poses
- Translate gestures into servo commands
- Send commands to the Arduino over serial
---

# Demo Video (Gesture Controller)
https://github.com/user-attachments/assets/9c3fa7d9-e886-4472-a17c-4c64cd100721
---

# **Requirements**:

## Arduino Controller
- PlatformIO IDE

## OpenCV Gesture Controller
- CMake
- OpenCV

## Materials Hardware
- Prices and quantity of all hardware componets, 3D-printed parts, and electronics used  in this project can be found in the following spreadsheet: [Click to View Full Materials List (Google Sheets)](<https://docs.google.com/spreadsheets/d/1fKG74xqfp3b54NL0HSdM__-Zx04nlzcWmwzY8FORccg/edit?usp=sharing>)

## 3D Printed Parts

Based on open-source models designed by: **[LimpSquid](<https://github.com/limpsquid>)**
