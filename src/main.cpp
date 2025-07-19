#include <Arduino.h>
#include "keyboard/KeyboardController.h"

// Controller instances
KeyboardController keyboardController;

// Menu state
enum MenuState {
    MAIN_MENU,
    KEYBOARD_CONTROL
};

MenuState currentState = MAIN_MENU;

void printMainMenu() {
    Serial.println("");
    Serial.println("===============================");
    Serial.println("    SERVO CONTROLLER MENU");
    Serial.println("===============================");
    Serial.println("");
    Serial.println("Available Controllers:");
    Serial.println("1. Keyboard Controller");
    Serial.println("2. Game Controller");
    Serial.println("3. Gesture Controller");
    Serial.println("");
    Serial.println("Enter your choice (1-3):");
    Serial.println("");
}

void handleMenuInput() {
    if (Serial.available() > 0) {
        char choice = Serial.read();
        
        switch (choice) {
            case '1':
                Serial.println("Starting Keyboard Controller...");
                Serial.println("");
                currentState = KEYBOARD_CONTROL;
                keyboardController.init();
                break;
                
            case '2':
                Serial.println("Game Controller is not yet implemented.");
                Serial.println("Please choose another option.");
                delay(1000);
                printMainMenu();
                break;
                
            case '3':
                Serial.println("Gesture Controller is not yet implemented.");
                Serial.println("Please choose another option.");
                delay(1000);
                printMainMenu();
                break;
                
            default:
                Serial.println("Invalid choice. Please enter 1, 2, or 3.");
                delay(500);
                printMainMenu();
                break;
        }
    }
}

void setup() {
    Serial.begin(9600);
    
    // Wait a moment for serial to initialize
    delay(1000);
    
    Serial.println("System Initializing...");
    delay(500);
    
    // Display main menu
    printMainMenu();
}

void loop() {
    switch (currentState) {
        case MAIN_MENU:
            handleMenuInput();
            break;
            
        case KEYBOARD_CONTROL:
            // Run keyboard controller, if it returns false, return to menu
            if (!keyboardController.update()) {
                currentState = MAIN_MENU;
                printMainMenu();
            }
            break;
    }
    
    delay(50);  // Small delay to prevent overwhelming the system
}