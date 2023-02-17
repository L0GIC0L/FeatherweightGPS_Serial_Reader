#include <iostream>
#include <cstring>
#include "serial.h"

string readLine(char incomingData[1], int &bytesRead) {
   string line = "";
    if(incomingData[0] != '\n') {
        do {
            #ifdef _WIN32
            if (ReadFile(hSerial, incomingData, 1, &bytesRead, NULL))
            {
                if ((incomingData[0] != '\n') && (incomingData[0] != '@')
                {
                    line += incomingData;
                }
            }
            #else

            bytesRead = read(serialPort, incomingData, 1);
            if (bytesRead > 0)
            {
                if((incomingData[0] != '\n') && (incomingData[0] != '@')) {
                    line += incomingData;
               }
            }
            #endif
        } while (incomingData[0] != '@');
    }
    return line;
}

int main(int argc, char* argv[])
{
    ////Normally this can be used to assign a port when launching the program
    // if (argc < 2)
    // {
    //     cerr << "Usage: serialport_example <serial port name>" << endl;
    //     return 1;
    // }

    //Change this line to "argv" configure the port
    if (!openSerialPort("/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_85734323430351B0C0A2-if00"))
    {
        return 1;
    }

    string line = "";
    int bytesRead;
    char incomingData[1] = "";
    bool lineGPS = false;

    while(lineGPS == false) {
        line = readLine(incomingData,bytesRead);
        cout << "IT WORKS";
        if (line.rfind("GPS") == 0) {
            cout << "Line found!!! dalfkasjdflk wow wowoww wowo" << endl << endl;
            lineGPS = true;
        }


    cout << "Behold the completed line: " << line << endl;
    sleep(1);
    }

    closeSerialPort();
    return 0;
}
