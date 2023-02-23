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
  string port = "/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_85734323430351B0C0A2-if00";

  // Map to store the parsed data
  std::multimap < std::string, std::string > parsed_data;

  //Set safe file directory
  std::string dir = "/home/nmiller/Projects/Brand New/FeatherweightGPS_Serial_Reader/src/";

  //Open the save file
  std::ofstream csv_file(dir + "data.csv");

  //Initialize input buffer
  std::string line = "";

  //Initiate Connection
  openSerialPort(port);

  //Read and List Data
  while (true) {
    line = readSerialPort("GPS_STAT");
    parseData(parsed_data, line);
    saveFile(parsed_data, csv_file);
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

  // rawOutput all the parsed data
  // for (const auto& pair : parsed_data) {
  //     cout << pair.first << ": " << pair.second << endl;
  // }

}

void saveFile(std::multimap<std::string, std::string>& parsed_data, std::ofstream& csv_file) {

  // Read the newest value
  const auto it = parsed_data.find("Hour");
  const auto m = parsed_data.find("Minute");
  const auto s = parsed_data.find("Seconds");
  const auto s3 = parsed_data.find("Milliseconds");
  const auto alt = parsed_data.find("Altitude");
  const auto la = parsed_data.find("Latitude");
  const auto ln = parsed_data.find("Longitude");
  const auto vx = parsed_data.find("VelocityX");
  const auto vy = parsed_data.find("VelocityY");
  const auto vz = parsed_data.find("VelocityZ");
  const auto sat = parsed_data.find("Satellite");

  if (it != parsed_data.end()) {
    csv_file << "{";
    std::cout << "Hour: " << it -> second << std::endl;
    csv_file << "Hour: " << it -> second;
  }
  if (m != parsed_data.end()) {
    std::cout << "Minute: " << m -> second << std::endl;
    csv_file << ", Minute: " << m -> second;
  }
  if (s != parsed_data.end()) {
    std::cout << "Second: " << s -> second << std::endl;
    csv_file << ", Second: " << s -> second;
  }
  if (s3 != parsed_data.end()) {
    std::cout << "Millisecond: " << s3 -> second << std::endl;
    csv_file << ", Millisecond: " << s3 -> second;
  }
  if (alt != parsed_data.end()) {
    std::cout << "Altitude: " << alt -> second << std::endl;
    csv_file << ", Altitude: " << alt -> second;
  }
  if (la != parsed_data.end()) {
    std::cout << "Latitude: " << la -> second << std::endl;
    csv_file << ", Latitude: " << la -> second;
  }
  if (la != parsed_data.end()) {
    std::cout << "Longitude: " << ln -> second << std::endl;
    csv_file << ", Longitude: " << ln -> second;
  }
  if (vx != parsed_data.end()) {
    std::cout << "VelocityX: " << vx -> second << std::endl;
    csv_file << ", VelocityX: " << vx -> second;
  }
  if (vy != parsed_data.end()) {
    std::cout << "VelocityY: " << vy -> second << std::endl;
    csv_file << ", VelocityY: " << vy -> second;
  }
  if (vz != parsed_data.end()) {
    std::cout << "VelocityZ: " << vz -> second << std::endl;
    csv_file << ", VelocityZ: " << vz -> second;
  }
  if (sat != parsed_data.end()) {
    std::cout << "Satelites: " << sat -> second << std::endl;
    csv_file << ", Satelites: " << sat -> second;
    csv_file << "}" << endl;
  }
  /*
  // Find all values for 'Timestamp'
  auto range = parsed_data.equal_range("Timestamp");
  for (auto it = range.first; it != range.second; ++it) {
  std::cout << "Values for 'Timestamp' are: " << it->second << std::endl;
  std::cout << r;
  }
  */
}
