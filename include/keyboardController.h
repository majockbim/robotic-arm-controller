#ifndef KEYBOARD_CONTROLLER_H
#define KEYBOARD_CONTROLLER_H

#include <Adafruit_PWMServoDriver.h>

// Call this once when entering keyboard control mode
void startKeyboardController(Adafruit_PWMServoDriver &pwm);

// Call this repeatedly while in keyboard control mode
// Return true if user wants to go back to the menu
bool runKeyboardController();

#endif
