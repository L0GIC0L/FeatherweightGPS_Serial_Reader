#ifdef _WIN32
#include <windows.h>
#include <string>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

#include <iostream>
#include <cstring>

using namespace std;

#ifdef _WIN32
HANDLE hSerial;
#else
int serialPort;
#endif

bool openSerialPort(std::string port) {
#ifdef _WIN32
    DCB dcbSerialParams = { 0 };

    // Disable buffer synchronization for cout
    ios_base::sync_with_stdio(false);

    // Open the serial port
    hSerial = CreateFileA(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        cout << "Error opening serial port!?!?!?!" << endl;
          return false;
    }

    // Set the parameters for the serial communication
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        cout << "Error getting the serial port state" << endl;
        return false;
    }

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        cout << "Error setting serial port state" << endl;
        return false;
    }

    cout << "Serial port opened successfully." << endl;
#else
    serialPort = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
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

string readSerialPort(string param)
{
#ifdef _WIN32
    DWORD dwBytesRead = 0;
    char incomingChar = '\0';
    string line;
    string buffer;

    DCB dcbSerialParams = { 0 };

     dwBytesRead = 0;
     while ( hSerial != INVALID_HANDLE_VALUE && GetCommState(hSerial, &dcbSerialParams) ) {

         if (!ReadFile(hSerial, &incomingChar, 1, &dwBytesRead, NULL)) {
             cout << "Error reading from the serial port" << endl;
         }

        if (dwBytesRead == 0) {
            // No data received, try again
            continue;
        }
        if (incomingChar == '\n') {
            line += incomingChar;
            if (line.find(param) != string::npos) {
                buffer += line;
                buffer += '\n';
            }
            line.clear();
        }
        else {
            line += incomingChar;
        }
        if (!buffer.empty() && buffer.back() == '\n') {
            return buffer;
        }
    }
#else
    char incomingChar[1] = "";
    int bytesRead;
    string buffer;
    string line;

    while (incomingChar[0] != '\n')
    {
        bytesRead = read(serialPort, incomingChar, 1);
        if (bytesRead > 0)
        {
            if (incomingChar[0] != '\n') {
                line += incomingChar[0];
            }
        }
    }

    if (line.find(param) != string::npos) {
      buffer += line + "\n";
    } else {
      buffer = "0";
    }

  if (buffer == "0") {
    buffer = "";
    return buffer;
  } else {
    return buffer;
  }
#endif
}
