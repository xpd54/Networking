#include "asio/buffer.hpp"
#include "asio/ip/address.hpp"
#include "asio/ip/tcp.hpp"
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <system_error>
#include <thread>
#include <vector>

std::vector<char> vBuffer(20 * 1024);
void GrabSomeData(asio::ip::tcp::socket &socket) {
  socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
                         [&](std::error_code ec, size_t length) {
                           if (!ec) {
                             std::cout << "\n\n Read " << length << " bytes"
                                       << '\n';
                             for (size_t i = 0; i < length; ++i)
                               std::cout << vBuffer[i];
                             GrabSomeData(socket);
                           }
                         });
}

int main() {
  asio::error_code ec;

  // create a context - essentially the platform specific interface.
  asio::io_context context;
  // have an address to connect with
  asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49 ", ec),
                                   80);
  // create a socket, the context will deliver the implementation
  asio::ip::tcp::socket socket(context);
  // tell socketto try and connect to the end point which we have defined
  socket.connect(endpoint, ec);

  if (!ec) {
    std::cout << "Connected!" << '\n';
  } else {
    std::cout << "Failed to connect to address: \n" << ec.message() << '\n';
  }

  if (socket.is_open()) {
    std::string sRequest = "GET /index.html HTTP/1.1\r\n"
                           "Host: example.com\r\n"
                           "Connection: close \r\n\r\n";
    /*If socket is open write some data in that socket with write_some*/
    socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);
    socket.wait(socket.wait_read);
    size_t bytes = socket.available();
    std::cout << "Bytes Available: " << bytes << '\n';

    if (bytes) {
      std::vector<char> vBuffer(bytes);
      /*If socket sends back some data read that data to a vector which holds
       * char*/
      socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
      for (auto &c : vBuffer)
        std::cout << c;
    }
  }

  std::this_thread::sleep_for(std::chrono::seconds(5));
  return 0;
}