#ifndef KEYBOARDCONTROLLER_H
#define KEYBOARDCONTROLLER_H

class KeyboardController {
public:
    KeyboardController();
    void init();
    bool update();  // Returns false when user wants to exit to menu
    
private:
    // Helper functions
    int* getCurrentPosition();
    int getCurrentMin();
    int getCurrentMax();
    void writeToCurrentServo(int position);
};

#endif