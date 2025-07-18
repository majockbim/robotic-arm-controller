#include <Arduino.h>
#include <Servo.h>

Servo servo1;  // Pin 9
Servo servo2;  // Pin 8
Servo servo3;  // Pin 7
Servo servo4; // Pin 6

int currentServo = 1;      
int stepSize = 5;        

// Individual servo positions
int servo1Position = 90;
int servo2Position = 90;
int servo3Position = 90;
int servo4Position = 90;

// Individual servo limits, yet to be determined.. defaults for now
int servo1Min = 20, servo1Max = 180;
int servo2Min = 0, servo2Max = 180;
int servo3Min = 0, servo3Max = 180;
int servo4Min = 0, servo4Max = 180;

// get current servo's position
int* getCurrentPosition() {
  switch (currentServo) {
    case 1: return &servo1Position;
    case 2: return &servo2Position;
    case 3: return &servo3Position;
    case 4: return &servo4Position;
    default: return &servo1Position;
  }
}

int getCurrentMin() {
  switch (currentServo) {
    case 1: return servo1Min;
    case 2: return servo2Min;
    case 3: return servo3Min;
    case 4: return servo4Min;
    default: return servo1Min;
  }
}

int getCurrentMax() {
  switch (currentServo) {
    case 1: return servo1Max;
    case 2: return servo2Max;
    case 3: return servo3Max;
    case 4: return servo4Max;
    default: return servo1Max;
  }
}

// write to current servo
void writeToCurrentServo(int position) {
  switch (currentServo) {
    case 1: servo1.write(position); break;
    case 2: servo2.write(position); break;
    case 3: servo3.write(position); break;
    case 4: servo4.write(position); break;
  }
}

void setup() {
  Serial.begin(9600);      // serial communication
  
  servo1.attach(9);        // Servo 1 on pin 9
  servo2.attach(8);        // Servo 2 on pin 8
  servo3.attach(7);        // Servo 3 on pin 7
  servo4.attach(6);        // Servo 4 on pin 6
  
  // Initialize all servos to center position
  servo1.write(servo1Position);
  servo2.write(servo2Position);
  servo3.write(servo3Position);
  servo4.write(servo4Position);
  
  Serial.println("=== MULTI-SERVO CONTROL READY ===");
  Serial.println("");
  Serial.println("SERVO SELECTION:");
  Serial.println("'1' - Control Servo 1 (Pin 9) [Limits: " + String(servo1Min) + "-" + String(servo1Max) + "]");
  Serial.println("'2' - Control Servo 2 (Pin 8) [Limits: " + String(servo2Min) + "-" + String(servo2Max) + "]");
  Serial.println("'3' - Control Servo 3 (Pin 7) [Limits: " + String(servo3Min) + "-" + String(servo3Max) + "]");
  Serial.println("'4' - Control Servo 4 (Pin 6) [Limits: " + String(servo4Min) + "-" + String(servo4Max) + "]");
  Serial.println("");
  Serial.println("MOVEMENT CONTROLS:");
  Serial.println("'A' - Move Left");
  Serial.println("'D' - Move Right");
  Serial.println("'S' - Reset to Center");
  Serial.println("'+' - Increase step size");
  Serial.println("'-' - Decrease step size");
  Serial.println("");
  Serial.println(">>> Currently controlling Servo " + String(currentServo) + " (Pin " + String(10 - currentServo) + ")");
  Serial.println(">>> Position: " + String(servo1Position) + " degrees");
  Serial.println(">>> Limits: " + String(getCurrentMin()) + "-" + String(getCurrentMax()) + " degrees");
  Serial.println("");
}

void loop() {
  // Check if data is available from serial port
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    command = toupper(command);
    
    bool positionChanged = false;
    int* currentPos = getCurrentPosition();
    
    switch (command) {
      case '1':
        currentServo = 1;
        Serial.println("");
        Serial.println(">>> SWITCHED TO SERVO 1 (Pin 9) <<<");
        Serial.println(">>> Position: " + String(servo1Position) + " degrees");
        Serial.println(">>> Limits: " + String(getCurrentMin()) + "-" + String(getCurrentMax()) + " degrees");
        Serial.println("");
        break;
        
      case '2': 
        currentServo = 2;
        Serial.println("");
        Serial.println(">>> SWITCHED TO SERVO 2 (Pin 8) <<<");
        Serial.println(">>> Position: " + String(servo2Position) + " degrees");
        Serial.println(">>> Limits: " + String(getCurrentMin()) + "-" + String(getCurrentMax()) + " degrees");
        Serial.println("");
        break;
        
      case '3':
        currentServo = 3;
        Serial.println("");
        Serial.println(">>> SWITCHED TO SERVO 3 (Pin 7) <<<");
        Serial.println(">>> Position: " + String(servo3Position) + " degrees");
        Serial.println(">>> Limits: " + String(getCurrentMin()) + "-" + String(getCurrentMax()) + " degrees");
        Serial.println("");
        break;

      case '4':
        currentServo = 4;
        Serial.println("");
        Serial.println(">>> SWITCHED TO SERVO 4 (Pin 6) <<<");
        Serial.println(">>> Position: " + String(servo4Position) + " degrees");
        Serial.println(">>> Limits: " + String(getCurrentMin()) + "-" + String(getCurrentMax()) + " degrees");
        Serial.println("");
        break;    
    
      case 'A':  // Move left
        if (*currentPos + stepSize <= getCurrentMax()) {
          *currentPos += stepSize;
          positionChanged = true;
        } else {
          Serial.println("Already at maximum position (" + String(getCurrentMax()) + ")!");
        }
        break;
        
      case 'D':  // Move right
        if (*currentPos - stepSize >= getCurrentMin()) {
          *currentPos -= stepSize;
          positionChanged = true;
        } else {
          Serial.println("Already at minimum position (" + String(getCurrentMin()) + ")!");
        }
        break;
        
      case 'S':  // Center
        *currentPos = 90;
        positionChanged = true;
        Serial.println("Servo " + String(currentServo) + " reset to center");
        break;
        
      case '+':  // Increase step size
        if (stepSize < 20) {
          stepSize += 1;
          Serial.println("Step size increased to: " + String(stepSize));
        } else {
          Serial.println("Step size already at maximum (20)!");
        }
        break;
        
      case '-':  // Decrease step size
        if (stepSize > 1) {
          stepSize -= 1;
          Serial.println("Step size decreased to: " + String(stepSize));
        } else {
          Serial.println("Step size already at minimum (1)!");
        }
        break;
        
      default:
        // Handle arrow key sequences
        // Left arrow sends: 27, 91, 68
        // Right arrow sends: 27, 91, 67
        if (command == 27) {  // ESC character (start of arrow key sequence)
          delay(10);  // Wait for rest of sequence
          if (Serial.available() >= 2) {
            Serial.read();  // Read '[' character
            char arrowCode = Serial.read();
            if (arrowCode == 68) {  // Left arrow
              if (*currentPos - stepSize >= getCurrentMin()) {
                *currentPos -= stepSize;
                positionChanged = true;
              }
            } else if (arrowCode == 67) {  // Right arrow
              if (*currentPos + stepSize <= getCurrentMax()) {
                *currentPos += stepSize;
                positionChanged = true;
              }
            }
          }
        }
        break;
    }
    
    // Update servo position if it changed
    if (positionChanged) {
      writeToCurrentServo(*currentPos);
      Serial.println("Servo " + String(currentServo) + " Position: " + String(*currentPos) + " degrees [" + String(getCurrentMin()) + "-" + String(getCurrentMax()) + "]");
    }
  }
  
  delay(50);  // delay to prevent overwhelming the serial buffer
}