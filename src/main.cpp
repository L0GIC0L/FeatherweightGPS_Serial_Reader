#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>

std::vector<std::string> getSerialPorts() {
  std::vector<std::string> serialPorts;

  TCHAR lpTargetPath[5000];
  DWORD test = QueryDosDevice(NULL, lpTargetPath, 5000);
  if (test != 0) {
    for (TCHAR *p = lpTargetPath; *p; p += _tcslen(p) + 1) {
      if (_tcsstr(p, _T("COM")) != NULL) {
        serialPorts.push_back(std::string(p));
      }
    }
  }

  return serialPorts;
}
#elif __linux__
std::vector<std::string> getSerialPorts() {
  std::vector<std::string> serialPorts;

    try {
  boost::asio::io_service io;
  boost::asio::serial_port device(io, "/dev/ttyACM0");

  if (device.is_open()) {
    serialPorts.push_back("/dev/ttyACM0");
  }
    } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }
  return serialPorts;
}
#else
std::vector<std::string> getSerialPorts() {
  std::vector<std::string> serialPorts;
  return serialPorts;
}
#endif

int main() {
  std::vector<std::string> ports = getSerialPorts();
  if (ports.empty()) {
    std::cout << "No serial ports found." << std::endl;
    return 0;
  }

  boost::asio::io_service io;
  boost::asio::serial_port port(io, ports[0]);
  port.set_option(boost::asio::serial_port_base::baud_rate(9600));

  try {
    for (;;) {
      boost::asio::streambuf buf;
      boost::asio::read_until(port, buf, '\n');

      std::string line;
      std::istream(&buf) >> line;

      std::cout << line << std::endl;
    }
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }

  return 0;
}
