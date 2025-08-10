#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "keyboardController.h"

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Menu state
enum Mode {
    MENU,
    KEYBOARD_CONTROL,
    GESTURE_CONTROL
};

Mode currentMode = MENU;

void showMainMenu() {
    Serial.println("=== ROBOTIC ARM CONTROLLER ===");
    Serial.println("1) Keyboard Controller");
    Serial.println("2) Gesture Controller (not implemented yet)");
    Serial.println("==============================");
}

void setup() {
    Serial.begin(9600);

    // Wait for Serial Monitor (up to 2s)
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime < 2000)) {;}

    // Init PCA9685
    Wire.begin();
    pwm.begin();
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(60); // 60 Hz for servos
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
                        Serial.println("Gesture Controller not implemented yet.");
                        showMainMenu();
                        break;
                    default:
                        Serial.println("Invalid option.");
                        showMainMenu();
                        break;
                }
            }
            break;

        case KEYBOARD_CONTROL:
            if (runKeyboardController()) { // true means go back to menu
                currentMode = MENU;
                showMainMenu();
            }
            break;

        case GESTURE_CONTROL:
            Serial.println("Gesture control mode placeholder.");
            currentMode = MENU;
            showMainMenu();
            break;
    }
}