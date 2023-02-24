#ifdef _WIN32

#include <windows.h>

#endif

#include <map>

#include <regex>

#include <sstream>

#include <iostream>

#include <fstream>

#include <string>

#include"serial.h"

void parseData(std::multimap < std::string, std::string >& , std::string);

void saveFile(std::multimap < std::string, std::string >& , std::ofstream& csv_file);

int main() {
  //Set the port directory (typically COM3 on windows)
  //string port = "/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_D201170B-if00-port0";
  string port = "/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_85734323430351B0C0A2-if00";

  // Map to store the parsed data
  std::multimap < std::string, std::string > parsed_data;

  //Set safe file directory
  std::string dir = "/home/nmiller/Projects/Brand New/FeatherweightGPS_Serial_Reader/src/";

  //Open the save file
  std::ofstream csv_file(dir + "data.csv");

  //Initialize input buffer
  std::string line = "";

   openSerialPort(port);

  //Read and List Data
  while (true) {
    line = readSerialPort("GPS_STAT");
    cout << endl;
    parseData(parsed_data, line);
    saveFile(parsed_data, csv_file);

    //sleep(2);
  }

  return 0;
}


//Functions
void parseData(std::multimap<std::string, std::string>& parsed_data, std::string line) {

  // Regular expression to match the timestamp and the remaining data
  std::regex data_regex(R"((\d{2}):(\d{2}):(\d{2})\.(\d{3}).*Alt\s+(\d+)\s+lt\s+([\+\-]?\d+\.\d+)\s+ln\s+([\+\-]?\d+\.\d+)\s+Vel\s+([\+\-]\d+)\s+([\+\-]\d+)\s+([\+\-]\d+)\s+Fix\s+(\d+))");

  // Parse the input string
  std::smatch match;
  if (std::regex_search(line, match, data_regex)) {
    // Extract data and insert it into the map
    parsed_data.insert({
      "Hour",
      match[1]
    });
    parsed_data.insert({
      "Minute",
      match[2]
    });
    parsed_data.insert({
      "Seconds",
      match[3]
    });
    parsed_data.insert({
      "Milliseconds",
      match[4]
    });
    parsed_data.insert({
      "Altitude",
      match[5]
    });
    parsed_data.insert({
      "Latitude",
      match[6]
    });
    parsed_data.insert({
      "Longitude",
      match[7]
    });
    parsed_data.insert({
      "VelocityX",
      match[8]
    });
    parsed_data.insert({
      "VelocityY",
      match[9]
    });
    parsed_data.insert({
      "VelocityZ",
      match[10]
    });
    parsed_data.insert({
      "Satellite",
      match[11]
    });
  }
}

void saveFile(std::multimap<std::string, std::string>& parsed_data, std::ofstream& csv_file) {

    // Output the latest value for each key
    csv_file << "{";
    for (const auto& key : {"Hour", "Minute", "Seconds", "Milliseconds", "Altitude", "Latitude", "Longitude", "VelocityX", "VelocityY", "VelocityZ", "Satellite"}) {
        std::cout << key << ": ";
        auto range = parsed_data.equal_range(key);
        auto it = range.second;
        if (it != range.first) {
            --it;
            std::cout << it->second;
            csv_file << it -> first << ": " << it -> second;
        }
        csv_file << ", ";
        std::cout << std::endl;
    }
    csv_file << "}" << endl;

  //rawOutput all the parsed data
  //for (const auto& pair : parsed_data) {
  //     cout << pair.first << ": " << pair.second << endl;
  //}
}
