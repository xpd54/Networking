#pragma once
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "network_common.h"
#include "network_message.h"
#include "network_thread_safe_queue.h"
#include <algorithm>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <system_error>
#include <thread>
xpd54_namespace_start template <typename T> class Network_Server {
public:
  Network_Server(uint16_t port)
      : m_asioAcceptor(m_asioContext,
                       asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

  virtual ~Network_Server() { Stop(); }

  bool Start() {
    try {
      WaitForClientConnection();
      m_threadContext = std::thread([this]() { m_asioContext.run(); });
    } catch (std::exception &e) {
      std::cerr << "[Server] Eception: " << e.what() << '\n';
      return false;
    }

    std::cout << "[Server] Started!" << '\n';
    return true;
  }

  void Stop() {
    m_asioContext.stop();
    // Tidy up context thread
    if (m_threadContext.joinable())
      m_threadContext.join();

    std::cout << "[Server] Stopped!" << '\n';
  }

  // Async - instruct asio to wait for connection
  void WaitForClientConnection() {
    m_asioAcceptor.async_accept([this](std::error_code &ec,
                                       asio::ip::tcp::socket socket) {
      if (!ec) {
        // get ip address
        std::cout << "[Server] New connection: " << socket.remote_endpoint()
                  << '\n';
        // create new connection
        std::shared_ptr<Connection<T>> new_connection =
            std::make_shared<Connection<T>>(Connection<T>::owner::server,
                                            m_asioContext, std::move(socket),
                                            m_qMessageIn);
      } else {
        // Error have happed during acceptance of connection
        std::cout << "[Server] New Connection Error: " << ec.message() << '\n';
      }

      // wait for another connection
      WaitForClientConnection();
    });
  }

  // Send a message to specifiec client
  void MessageClient(std::shared_ptr<Connection<T>> client, Message<T> &msg) {}

  void
  MessageAllClient(const Message<T> &msg,
                   std::shared_ptr<Connection<T>> ignore_client = nullptr) {}

protected:
  virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client) {
    return false;
  }

  virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client) {}

  virtual void OnMessage(std::shared_ptr<Connection<T>> client,
                         Message<T> &msg) {}

  // Thread safe queue for all incoming mesage
  thread_safe_queue<Owned_message<T>> m_qMessageIn;

  asio::io_context m_asioContext;
  std::thread m_threadContext;

  asio::ip::tcp::acceptor m_asioAcceptor;

  // Client will be indetified in the "wider system" via a ID

  uint32_t nIDCounter = 10000;
};

xpd54_namespace_end