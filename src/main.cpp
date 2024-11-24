#include "asio/ip/address.hpp"
#include "asio/ip/tcp.hpp"
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <chrono>
#include <iostream>
#include <thread>
int main() {
  asio::error_code ec;

  // create a context - essentially the platform specific interface.
  asio::io_context context;
  asio::ip::tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1", ec), 80);
  // create a socket, the context will deliver the implementation
  asio::ip::tcp::socket socket(context);
  // tell socketto try and connect

  socket.connect(endpoint, ec);

  if (!ec) {
    std::cout << "Connected!" << '\n';
  } else {
    std::cout << "Failed to connect to address: \n" << ec.message() << '\n';
  }

  std::this_thread::sleep_for(std::chrono::seconds(5));
  return 0;
}