#pragma once
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "network_common.h"
#include "network_message.h"
#include "network_thread_safe_queue.h"
#include <__algorithm/remove.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <exception>
#include <iostream>
#include <memory>
#include <sys/_types/_pid_t.h>
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
      wait_for_client_connection();
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
  void wait_for_client_connection() {
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
                                            m_qMessagesIn);
        // Give the user server a change to deny connection
        if (on_client_connect(new_connection)) {
          m_deqConnections.push_back(new_connection);
          m_deqConnections.back()->connect_to_client(nIDCounter++);
          std::cout << "[" << m_deqConnections.back()->GetId() << "]"
                    << " Connection Approved" << '\n';
        } else {
          std::cout << "[-----] Connection Denied" << '\n';
        }
      } else {
        // Error have happed during acceptance of connection
        std::cout << "[Server] New Connection Error: " << ec.message() << '\n';
      }

      // wait for another connection
      wait_for_client_connection();
    });
  }

  // Send a message to specifiec client
  void message_client(std::shared_ptr<Connection<T>> client, Message<T> &msg) {
    if (client && client.is_connected()) {
      client->send(msg);
    } else {
      on_client_disconnect(client);
      client.reset();
      m_deqConnections.erase(
          std::remove(m_deqConnections.begin(), m_deqConnections.end(), client),
          m_deqConnections.end());
    }
  }

  void message_all_client(const Message<T> &msg,
                          std::shared_ptr<Connection<T>> ignore = nullptr) {
    bool invlidClientExists = false;
    for (auto &client : m_deqConnections) {
      if (client && client->is_connected()) {
        if (client != ignore) {
          client->send(msg);
        } else {
          on_client_disconnect(client);
          client.reset();
          invlidClientExists = true;
        }
      }
    }
    if (invlidClientExists) {
      m_deqConnections.erase(std::remove(m_deqConnections.begin(),
                                         m_deqConnections.end(), nullptr),
                             m_deqConnections.end());
    }
  }

  void update(size_t nMaxMessages = -1) {
    size_t nMessageCount = 0;
    while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty()) {
      auto msg = m_qMessagesIn.pop_front();
      on_message(msg.remote, msg.msg);
      nMessageCount++;
    }
  }

protected:
  virtual bool on_client_connect(std::shared_ptr<Connection<T>> client) {
    return false;
  }

  virtual void on_client_disconnect(std::shared_ptr<Connection<T>> client) {}

  virtual void on_message(std::shared_ptr<Connection<T>> client,
                          Message<T> &msg) {}

  // Thread safe queue for all incoming mesage
  thread_safe_queue<Owned_message<T>> m_qMessagesIn;
  // Container of active validated connecitons
  std::deque<std::shared_ptr<Connection<T>>> m_deqConnections;

  asio::io_context m_asioContext;
  std::thread m_threadContext;

  asio::ip::tcp::acceptor m_asioAcceptor;

  // Client will be indetified in the "wider system" via a ID

  uint32_t nIDCounter = 10000;
};

xpd54_namespace_end