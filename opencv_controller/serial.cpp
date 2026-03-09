// serial.cpp
#include "serial.h"
#include <iostream>

SerialPort::SerialPort() : hSerial(INVALID_HANDLE_VALUE), connected(false) {}

SerialPort::~SerialPort() {
    disconnect();
}

bool SerialPort::connect(const std::string& portName, int baudRate) {
    if (connected) {
        std::cout << "Already connected!" << std::endl;
        return true;
    }

    hSerial = CreateFileA(portName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Could not open port " << portName << std::endl;
        return false;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error: Could not get serial state" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error: Could not set serial state" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    // set timeouts
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Warning: Could not set timeouts" << std::endl;
        // not fatal
    }

    // purge any junk data in buffer
    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

    connected = true;
    std::cout << "Connected to " << portName << " at " << baudRate << " baud" << std::endl;
    return true;
}

void SerialPort::disconnect() {
    if (connected && hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        connected = false;
        std::cout << "Serial port disconnected" << std::endl;
    }
}

bool SerialPort::sendCommand(char command) {
    if (!connected) return false;

    DWORD bytesWritten = 0;
    const int MAX_RETRIES = 3;

    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        if (WriteFile(hSerial, &command, 1, &bytesWritten, NULL) && bytesWritten == 1) {
            // ensure data is flushed out of OS buffer
            FlushFileBuffers(hSerial);
            return true;
        }

        // small backoff and retry
        Sleep(50);
    }

    std::cerr << "Error writing to serial port after retries" << std::endl;
    return false;
}

bool SerialPort::sendString(const std::string& data) {
    if (!connected) return false;

    DWORD bytesWritten = 0;
    if (!WriteFile(hSerial, data.c_str(), static_cast<DWORD>(data.length()), &bytesWritten, NULL) || bytesWritten != data.length()) {
        std::cerr << "Error writing string to serial port" << std::endl;
        return false;
    }

    FlushFileBuffers(hSerial);
    return true;
}