#include "keyboardController.h"
#include <Arduino.h>

static Adafruit_PWMServoDriver *pwmPtr = nullptr;
static uint8_t servoNum = 0;

#define SERVOMIN 150
#define SERVOMAX 600
#define NUM_SERVOS 6

// Custom starting positions
static uint16_t startPositions[NUM_SERVOS] = {
    430, // Servo 0
    190, // Servo 1
    510, // Servo 2
    440, // Servo 3
    600, // Servo 4
    150  // Servo 5 (opposite of servo 4)
};

// Track each servo's pulse length individually
static uint16_t pulseLengths[NUM_SERVOS];

// Step size
static uint16_t stepSize = 5;

void printControls() {
    Serial.println("Controls:");
    Serial.println("  0-5 : Select servo");
    Serial.println("  D / +   : Increase angle");
    Serial.println("  A / -   : Decrease angle");
    Serial.println("  s<number> : Set step size (e.g., s10 for 10)");
    Serial.println("  m   : Return to main menu");
}

void startKeyboardController(Adafruit_PWMServoDriver &pwm) {
    pwmPtr = &pwm;
    servoNum = 0;

    // Initialize all servos to their custom start positions
    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        pulseLengths[i] = startPositions[i];
        pwmPtr->setPWM(i, 0, pulseLengths[i]);
    }

    Serial.println("Keyboard controller started.");
    printControls();
}

bool runKeyboardController() {
    if (!pwmPtr) return false; // not started yet

    while (Serial.available()) {
        char c = Serial.read();

        // Select servo (0-5)
        if (c >= '0' && c <= '5') {
            servoNum = c - '0';
            Serial.print("Selected servo: ");
            Serial.println(servoNum);
            Serial.print("Current position: ");
            Serial.println(pulseLengths[servoNum]);
        }
        // Increase position (D or +)
        else if (c == 'D' || c == 'd' || c == '+') {
            if (pulseLengths[servoNum] < SERVOMAX) pulseLengths[servoNum] += stepSize;
            if (pulseLengths[servoNum] > SERVOMAX) pulseLengths[servoNum] = SERVOMAX;
            pwmPtr->setPWM(servoNum, 0, pulseLengths[servoNum]);

            Serial.print("Servo "); Serial.print(servoNum);
            Serial.print(" position: "); Serial.println(pulseLengths[servoNum]);

            // Linked servo logic for servo 4 -> servo 5
            if (servoNum == 4) {
                int diff = pulseLengths[4] - startPositions[4];
                pulseLengths[5] = startPositions[5] - diff;
                if (pulseLengths[5] > SERVOMAX) pulseLengths[5] = SERVOMAX;
                if (pulseLengths[5] < SERVOMIN) pulseLengths[5] = SERVOMIN;
                pwmPtr->setPWM(5, 0, pulseLengths[5]);

                Serial.print("Servo 5 position (linked): ");
                Serial.println(pulseLengths[5]);
            }
        }
        // Decrease position (A or -)
        else if (c == 'A' || c == 'a' || c == '-') {
            if (pulseLengths[servoNum] > SERVOMIN) pulseLengths[servoNum] -= stepSize;
            if (pulseLengths[servoNum] < SERVOMIN) pulseLengths[servoNum] = SERVOMIN;
            pwmPtr->setPWM(servoNum, 0, pulseLengths[servoNum]);

            Serial.print("Servo "); Serial.print(servoNum);
            Serial.print(" position: "); Serial.println(pulseLengths[servoNum]);

            // Linked servo logic for servo 4 -> servo 5
            if (servoNum == 4) {
                int diff = pulseLengths[4] - startPositions[4];
                pulseLengths[5] = startPositions[5] - diff;
                if (pulseLengths[5] > SERVOMAX) pulseLengths[5] = SERVOMAX;
                if (pulseLengths[5] < SERVOMIN) pulseLengths[5] = SERVOMIN;
                pwmPtr->setPWM(5, 0, pulseLengths[5]);

                Serial.print("Servo 5 position (linked): ");
                Serial.println(pulseLengths[5]);
            }
        }
        // Step size command
        else if (c == 's' || c == 'S') {
            String numStr;
            unsigned long start = millis();
            while (millis() - start < 50) { // short timeout
                if (Serial.available()) {
                    char digit = Serial.read();
                    if (isdigit(digit)) {
                        numStr += digit;
                    } else {
                        break;
                    }
                }
            }
            int newStep = numStr.toInt();
            if (newStep > 0) {
                stepSize = newStep;
                Serial.print("Step size changed to: ");
                Serial.println(stepSize);
            } else {
                Serial.println("Invalid step size.");
            }
        }
        // Return to menu
        else if (c == 'm' || c == 'M') {
            Serial.println("Returning to main menu...");
            return true;
        }
    }
    return false;
}
