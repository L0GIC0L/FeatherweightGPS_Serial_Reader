#include "lib/serialib.h"
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <vector>

bool
ifolder (const char *folder)
{
  // folder = "C:\\Users\\SaMaN\\Desktop\\Ppln";
  struct stat sb;

  if (stat (folder, &sb) == 0 && S_ISDIR (sb.st_mode))
    {
      return true;
    }
  else
    {
      return false;
    }
}

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>

std::vector<std::string>
getSerialPorts ()
{
  std::vector<std::string> serialPorts;

  TCHAR lpTargetPath[5000];
  DWORD test = QueryDosDevice (NULL, lpTargetPath, 5000);
  if (test != 0)
    {
      for (TCHAR *p = lpTargetPath; *p; p += _tcslen (p) + 1)
        {
          if (_tcsstr (p, _T("COM")) != NULL)
            {
              serialPorts.push_back (std::string (p));
            }
        }
    }

  return serialPorts;
}

#elif __linux__
std::vector<std::string>
getSerialPorts ()
{
  std::vector<std::string> serialPorts;

  if (ifolder (
          "/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_D201170B-if00-port0")
      == false)
    {
      return serialPorts;
    }
  else
    {
      serialPorts.push_back (
          "/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_D201170B-if00-port0");
      return serialPorts;
    }
}

#else
std::vector<std::string>
getSerialPorts ()
{
  std::vector<std::string> serialPorts;
  return serialPorts;
}
#endif

int
main ()
{
  std::vector<std::string> serial_ports = getSerialPorts ();

  if (serial_ports.empty ())
    {
      std::cout << "No serial ports found." << std::endl;
      return 0;
    }
  else
    {
      std::cout << "Port found! Connecting to: " << serial_ports[0].c_str ()
                << std::endl;
    }

  const char *primary_port = serial_ports[0].c_str ();

  serialib serial;
  serial.openDevice (primary_port, 115200, SERIAL_DATABITS_8,
                     SERIAL_PARITY_NONE, SERIAL_STOPBITS_1);

  std::string data;
  // std::map<std::string, std::string> gpsData;

  for (;;)
    {
      char c = 0;
      data = "";

      do
        {
          serial.readString (&c, '\n', 1024, 0);
          data += c;
        }
      while (c != '\n');

      std::cout << data;
      std::cout << "The string length is: " << data.length () << std::endl;
      std::cout << std::endl;

      // Basic code to break up the variables and store them in a map
      //    std::istringstream iss(data);
      //    std::string key, value;

      // Extract the first word "BATT_BLE" and discard it
      //   iss >> key;

      //  int i = 0;
      //  while (iss >> value) {
      // Store each variable after "BATT_BLE" in the map with a key starting
      // from 0
      //  gpsData[std::to_string(i++)] = value;
      //  }
      //    for (int j = 0; j < i; j++) {
      //      std::cout << gpsData[std::to_string(j)] << std::endl;
      //    }
    }
  return 0;
}
