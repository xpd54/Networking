#pragma once
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <system_error>

#include "asio/buffer.hpp"
#include "asio/connect.hpp"
#include "asio/post.hpp"
#include "asio/read.hpp"
#include "asio/write.hpp"
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

  void send(const Message<T> &msg) {
    asio::post(m_asioContext, [this, msg]() {
      bool is_writing_message = !m_qMessagesOut.empty();
      m_qMessagesOut.push_back(msg);
      if (!is_writing_message) {
        write_header();
      }
    });
  }

  void connect_to_server(const asio::ip::tcp::resolver::results_type &endpoints) {
    // only client can connect to servers
    if (m_nOwerType == Owner::client) {
      asio::async_connect(m_socket, endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
        if (!ec) {
          read_header();
        }
      });
    }
  }

  void connect_to_client(uint32_t uid = 0) {
    // only server connection can connect to client
    if (m_nOwerType == Owner::server) {
      if (m_socket.is_open()) {
        id = uid;
        read_header();
      }
    }
  }

  void disconnect() {
    if (is_connected()) {
      asio::post(m_asioContext, [this]() { m_socket.close(); });
    }
  }

  bool is_connected() const { return m_socket.is_open(); }

private:
  // async:- ready to read a message header
  void read_header() {
    asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
                     [this](std::error_code ec, std::size_t length) {
                       if (!ec) {
                         if (m_msgTemporaryIn.header.size > 0) {
                           m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                           read_body();
                         } else {
                           add_to_incomming_message_queue();
                         }
                       } else {
                         std::cout << "[" << id << "]"
                                   << " Read Header fail." << '\n';
                         m_socket.close();
                       }
                     });
  }
  // async:- ready to read a message body
  void read_body() {
    asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
                     [this](std::error_code ec, std::size_t length) {
                       if (!ec) {
                         add_to_incomming_message_queue();
                       } else {
                         std::cout << "[" << id << "]"
                                   << " Read Body fail." << '\n';
                         m_socket.close();
                       }
                     });
  }

  // async:- ready to write a message header
  void write_header() {
    asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
                      [this](std::error_code ec, std::size_t length) {
                        if (!ec) {
                          if (m_qMessagesOut.front().body.size() > 0) {
                            write_body();
                          } else {
                            // as no body to send remove front message
                            m_qMessagesOut.pop_front();
                            // Also check if whole message queue is empty or what
                            if (m_qMessagesOut.empty()) {
                              write_header();
                            }
                          }
                        } else {
                          std::cout << "[" << id << "]"
                                    << " Write header fail." << '\n';
                          m_socket.close();
                        }
                      });
  }

  // async:- ready to wite a message body
  void write_body() {
    asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
                      [this](std::error_code ec, std::size_t length) {
                        if (!ec) {
                          // if successfull to write and no errro pop the front message
                          m_qMessagesOut.pop_front();
                          if (!m_qMessagesOut.empty()) {
                            write_header();
                          }
                        } else {
                          std::cout << "[" << id << "]"
                                    << " Write body fail." << '\n';
                          m_socket.close();
                        }
                      });
  }

  void add_to_incomming_message_queue() {
    if (m_nOwerType == Owner::server) {
      m_qMessageIn.push_back({this->shared_from_this(), m_msgTemporaryIn});
    } else {
      m_qMessageIn.push_back({nullptr, m_msgTemporaryIn});
    }
    read_header();
  }

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
  // temporary message buffer which holds read header.
  Message<T> m_msgTemporaryIn;
  Owner m_nOwerType = Owner::server;
  uint32_t id = 0;
};

xpd54_namespace_end