#ifndef SERIAL_H
#define SERIAL_H

#include <string>
#include <windows.h>

class SerialPort {
public:
    SerialPort();
    ~SerialPort();
    
    bool connect(const std::string& portName, int baudRate = 9600);
    void disconnect();
    bool isConnected() const { return connected; }
    
    bool sendCommand(char command);
    bool sendString(const std::string& data);
    
private:
    HANDLE hSerial;
    bool connected;
};

#endif // SERIAL_H