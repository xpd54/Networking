#pragma once
#include "network_common.h"
#include "network_message.h"
#include "network_thread_safe_queue.h"
#include <cstdint>
#include <memory>
xpd54_namespace_start template <typename T> class Network_Server {
public:
  Network_Server(uint16_t port) {}

  virtual ~Network_Server() {}

  bool Start() {}

  void Stop() {}

  // Async - instructo asio to wait for connection
  void WaitForClientConnection() {}

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
  thread_safe_queue<Owned_message<T>> m_messageIn;
};

xpd54_namespace_end