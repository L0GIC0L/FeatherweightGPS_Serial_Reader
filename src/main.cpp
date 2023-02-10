#include <iostream>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

using namespace std;

#ifdef _WIN32
HANDLE hSerial;
#else
int serialPort;
#endif

bool openSerialPort(string portName)
{
#ifdef _WIN32
    hSerial = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        cerr << "Error opening serial port" << endl;
        return false;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        cerr << "Error getting serial port state" << endl;
        return false;
    }

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        cerr << "Error setting serial port state" << endl;
        return false;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        cerr << "Error setting serial port timeouts" << endl;
        return false;
    }
#else


    serialPort = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (serialPort == -1)
    {
        cerr << "Error opening serial port" << endl;
        return false;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(serialPort, &tty) != 0)
    {
        cerr << "Error getting serial port attributes" << endl;
        return false;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~CRTSCTS;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 5;
        tty.c_cflag |= CREAD | CLOCAL;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;

    if (tcsetattr(serialPort, TCSANOW, &tty) != 0)
    {
        cerr << "Error setting serial port attributes" << endl;
        return false;
    }
#endif
    return true;
}

void closeSerialPort()
{
#ifdef _WIN32
    CloseHandle(hSerial);
#else
    close(serialPort);
#endif
}

int main(int argc, char* argv[])
{
    // if (argc < 2)
    // {
    //     cerr << "Usage: serialport_example <serial port name>" << endl;
    //     return 1;
    // }

    if (!openSerialPort("/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_85734323430351B0C0A2-if00"))
    {
        return 1;
    }

#ifdef _WIN32
    char incomingData[256] = "";
    DWORD bytesRead;
    while (true)
    {
        if (ReadFile(hSerial, incomingData, 255, &bytesRead, NULL))
        {
            cout << incomingData;
        }
    }

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


