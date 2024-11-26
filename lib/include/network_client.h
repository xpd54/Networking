#pragma once
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "network_common.h"
#include "network_connection.h"
#include "network_message.h"
#include "network_thread_safe_queue.h"
xpd54_namespace_start template <typename T> class Network_Client {
public:
  Network_Client() : m_socket(m_context) {
    // Initialize
  }

  virtual ~Network_Client() { disconnect(); }

  // connect to given ip address and port of server
  bool connect(const std::string &host, const uint32_t port) {
    try {
      // Resolve the hostname to ip address before connect to server
      asio::ip::tcp::resolver resolver(m_context);
      asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

      // create a connection
      m_connection = std::make_unique<Connection<T>>(Connection<T>::Owner::client, m_context,
                                                     asio::ip::tcp::socket(m_context), m_qMessagesIn);

      // Tell the connection object to connect to server
      m_connection->connect_to_server(endpoints);

      // Start the context thread
      thread_context = std::thread([this]() { m_context.run(); });
    } catch (std::exception &e) {
      std::cerr << "Client Exception: " << e.what() << '\n';
      return false;
    }
    return true;
  }

  void disconnect() {
    if (is_connected()) {
      m_connection->disconnect();
    }

    // even not connected we still done with asio context
    m_context.stop();

    // Thread work is done for that context as well
    if (thread_context.joinable())
      thread_context.join();

    // Destroy the connection object
    m_connection.release();
  }

  bool is_connected() {
    if (m_connection) {
      return m_connection->is_connected();
    } else {
      return false;
    }
  }

  void send(const Message<T> &msg) {
    if (is_connected())
      m_connection->send(msg);
  }

  thread_safe_queue<Owned_message<T>> &incoming() { return m_qMessagesIn; }

protected:
  /* Client will setup the connection. The negotiation between server and client
   * will happen to setup the connection. Connection will only exist if
   * connection is valid. The resposibility of establising the valid connection
   * is on connection*/
  asio::io_context m_context;
  // it will need thread of it's own to execute it's work
  std::thread thread_context;
  // The socket which connects to the server
  asio::ip::tcp::socket m_socket;
  // If valid connection get createed it will hold single instance of it.
  // (unique_ptr)
  std::unique_ptr<Connection<T>> m_connection;

private:
  // incoming messages from server which connection have as reference.
  thread_safe_queue<Owned_message<T>> m_qMessagesIn;
};
xpd54_namespace_end