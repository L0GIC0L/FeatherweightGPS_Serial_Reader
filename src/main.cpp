#ifdef _WIN32

#include <windows.h>
#endif

#include <map>
#include <regex>
#include <sstream>
#include <iostream>
#include <string>

#include"serial.h"

struct GPSData {
    string timestamp;
    double latitude;
    double longitude;
    int altitude;
};

using namespace std;

int main()
{
    string port = "COM3";

    openSerialPort(port);
    while (true) {
        string line = "";
        line = readSerialPort("GPS_STAT");

    // Regular expression to match the timestamp and the remaining data
    std::regex data_regex(R"((\d{2}:\d{2}:\d{2}\.\d{3}).*(Alt\s+(\d+)\s+lt\s+([\+\-]?\d+\.\d+)\s+ln\s+([\+\-]?\d+\.\d+)))");

    // Map to store the parsed data
    std::map<std::string, std::string> parsed_data;

    // Parse the input string
    std::smatch match;
    if (std::regex_search(line, match, data_regex)) {
        parsed_data["Timestamp"] = match[1];
        parsed_data["Altitude"] = match[3];
        parsed_data["Latitude"] = match[4];
        parsed_data["Longitude"] = match[5];
    }

    // Output the parsed data
    for (const auto& pair : parsed_data) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }

}

    return 0;
}
