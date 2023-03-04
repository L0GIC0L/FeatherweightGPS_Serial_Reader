#ifdef _WIN32

#include <windows.h>

#endif

#include <map>

#include <iostream>

#include <string>

#include "functions.h"

int main() {
    //Set the port directory (typically COM3 on windows)
    //string port = "/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_D201170B-if00-port0";
    std::string port = "/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_85734323430351B0C0A2-if00";

    // Map to store the parsed data
    std::multimap < std::string, std::string > parsed_data;

    //Set safe file directory
    std::string dir = R"(/home/nmiller/)";

    //Open the save file
    std::ofstream csv_file(dir + "data.csv");

    //Initialize input buffer
    std::string line = "";

    openSerialPort(port);

    //Read and List Data
    while (true) {
        line = readSerialPort("GPS_STAT");
        parseData(parsed_data, line);
        saveFile(parsed_data, csv_file);
        //std::cout << retrieveLatest(parsed_data, R"(Seconds)") << std::endl;
    }

    return 0;
}



