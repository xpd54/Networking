#pragma once
#include <algorithm>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <cstdint>
#include <memory>

#include "network_common.h"
#include "network_message.h"
#include "network_thread_safe_queue.h"
xpd54_namespace_start template <typename T> class Connection : public std::enable_shared_from_this<Connection<T>> {
public:
  // A connection can be hold by a server or client same object get used in both places
  enum class Owner {
    server,
    client,
  };

  /*Creticle part here get initialized and non creticle get assigned*/
  Connection(Owner parent, asio::io_context &asioContext, asio::ip::tcp::socket socket,
             thread_safe_queue<Owned_message<T>> &incoming_message_queue)
      : m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessageIn(incoming_message_queue) {
    m_nOwerType = parent;
  }

  virtual ~Connection() {}

  uint32_t get_id() const { return id; }
  bool send(const Message<T> &msg);
  bool connect_to_server();

  void connect_to_client(uint32_t uid = 0) {
    if (m_nOwerType == Owner::server) {
      if (m_socket.is_open()) {
        id = uid;
      }
    }
  }

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
  thread_safe_queue<Owned_message<T>> &m_qMessageIn;
  Owner m_nOwerType = Owner::server;
  uint32_t id = 0;
};

xpd54_namespace_end