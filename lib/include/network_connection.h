#pragma once
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "network_common.h"
#include "network_message.h"
#include "network_thread_safe_queue.h"
#include <memory>
xpd54_namespace_start template <typename T>
class Connection : public std::enable_shared_from_this<Connection<T>> {
public:
  Connection() {}
  virtual ~Connection() {}

  bool send(const Message<T> &msg);
  bool connect_to_server();
  bool disconnect();
  bool is_connected() const;

protected:
  // each connection has a unique socket to a remote
  asio::ip::tcp::socket m_socket;

  // single context for all asio instance
  asio::io_context &m_asioContext;

  // This queue holds all message to be sent to the remote side
  // Will check this queue to send message
  thread_safe_queue<Message<T>> m_qMessagesOut;

  // This should hold all the messages which have received.
  // This is just a reference as owner of this connection whould provide the
  // queue.
  thread_safe_queue<Owned_message<T>> &m_qMessageIN;
};

xpd54_namespace_end