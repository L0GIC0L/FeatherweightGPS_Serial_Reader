#include <iostream>
#include <cstring>
#include "serial.h"

int main(int argc, char* argv[])
{
    ////Normally this can be used to assign a port when launching the program
    // if (argc < 2)
    // {
    //     cerr << "Usage: serialport_example <serial port name>" << endl;
    //     return 1;
    // }

    //Change this line to configure the port
    if (!openSerialPort("COM3"))
    {
        return 1;
    }

#ifdef _WIN32
    char incomingData[1] = "";
    string line = "";
    DWORD bytesRead;
    while (incomingData[0] != '\n')
    {
        if (ReadFile(hSerial, incomingData, 1, &bytesRead, NULL))
        {
            line += incomingData[0];
        }

    }
    cout << "Behold the completed line: " << line << endl;

#else
    char incomingData[1] = "";
    int bytesRead;
    string line = "";
    while (incomingData[0] != '\n')
    {
        bytesRead = read(serialPort, incomingData, 1);
        if (bytesRead > 0)
        {
            if (incomingData[0] != '\n') {
                line += incomingData[0];
            }
        }
    }
    cout << "Behold the completed line: " << line << endl;
#endif
    closeSerialPort();
    return 0;
}
