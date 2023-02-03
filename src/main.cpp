#include <iostream>
#include <boost/asio.hpp>

int main() {
  // Create a serial port object using the default constructor
  boost::asio::io_service io;
  boost::asio::serial_port port(io);

  // Open the serial port
  port.open("/dev/ttyACM0");

  // Set the baud rate, data bits, and other properties of the serial connection
  port.set_option(boost::asio::serial_port_base::baud_rate(115200));
  port.set_option(boost::asio::serial_port_base::character_size(8));
  port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
  port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
  port.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

  // Read data from the serial port
  boost::asio::streambuf buf;
  boost::asio::read_until(port, buf, '\n');

  // Convert the data to a string and print it
  std::string data = boost::asio::buffer_cast<const char*>(buf.data());
  std::cout << data << std::endl;

  // Close the serial port
  port.close();

  return 0;
}

