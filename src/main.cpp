#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "keyboardController.h"

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVO_PIN 0 // channel 0 PCA9685
#define SERVOMIN 150
#define SERVOMAX 600
#define NUM_SERVOS 6
const int STEP_SIZE = 10;           // Move 10° per gesture
#define MOVEMENT_INTERVAL 50

enum Mode { MENU, KEYBOARD_CONTROL, GESTURE_CONTROL };
Mode currentMode = MENU;

uint8_t currentServo = 0;
uint16_t servoPulseLengths[NUM_SERVOS] = {430, 190, 510, 440, 600, 150};
unsigned long lastServoSwitchTime = 0;
unsigned long lastMovementTime = 0;
const unsigned long SERVO_SWITCH_INTERVAL = 2000;

enum MovementDirection { STOPPED, FORWARD, BACKWARD };
MovementDirection currentMovement = STOPPED;
unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 200;

void handleStringCommand() {
    // Read the full incoming line until newline character
    String inputString = Serial.readStringUntil('\n');

    // Check if the command starts with 'A' (Pulse Command)
    if (inputString.startsWith("A")) {
        // Extract the numerical part of the string (e.g., "375" from "A375")
        String pulseStr = inputString.substring(1); 
        int newPulse = pulseStr.toInt();

        // Safety checks (using existing constants)
        if (newPulse < SERVOMIN) newPulse = SERVOMIN;
        if (newPulse > SERVOMAX) newPulse = SERVOMAX;
        
        // This command only controls Servo 0
        pwm.setPWM(SERVO_PIN, 0, newPulse);
        
        // Update the stored value for Servo 0 (useful if you return to Keyboard mode later)
        servoPulseLengths[SERVO_PIN] = newPulse;

        // Debugging feedback
        Serial.print("Rx Pulse (Ch 0): ");
        Serial.println(newPulse);
    }
}

void showMainMenu() {
    Serial.println("=== ROBOTIC ARM CONTROLLER ===");
    Serial.println("1) Keyboard Controller");
    Serial.println("2) Gesture Controller");
    Serial.println("==============================");
}

void setup() {
    Serial.begin(9600);
    
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime < 2000)) {;}

    Wire.begin();
    pwm.begin();
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(60);
    delay(10);

    Serial.println("I2C initialized successfully");
    Serial.println();
    showMainMenu();
}

void loop() {
    switch (currentMode) {
        case MENU:
            if (Serial.available()) {
                char choice = Serial.read();
                switch (choice) {
                    case '1':
                        Serial.println("Keyboard Controller selected.");
                        currentMode = KEYBOARD_CONTROL;
                        startKeyboardController(pwm);
                        break;
                    case '2':
                        Serial.println("Gesture Controller activated. Waiting for PULSE commands...");
                        currentMode = GESTURE_CONTROL;
                        // Set the initial pulse for the test servo
                        pwm.setPWM(SERVO_PIN, 0, servoPulseLengths[SERVO_PIN]); 
                        // Remove the call to startGestureController()
                        break;
                    default:
                        Serial.println("Invalid option.");
                        showMainMenu();
                        break;
                }
            }
            break;

        case KEYBOARD_CONTROL:
            if (runKeyboardController()) {
                currentMode = MENU;
                showMainMenu();
            }
            break;

        case GESTURE_CONTROL:

            if (Serial.available()) {
                handleStringCommand();
            }

            break;
    }
}