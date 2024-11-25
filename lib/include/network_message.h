#pragma once
#include "network_common.h"
#include <cstddef>
#include <cstring>
#include <ostream>
#include <type_traits>

namespace xpd54 {
namespace network {
template <typename T> struct message_header {
  T id{};
  uint32_t size = 0;
};

template <typename T> struct message {
  message_header<T> header{};
  std::vector<uint8_t> body;

  inline size_t size() const { return sizeof(message_header<T>) + body.size(); }
  friend std::ostream &operator<<(std::ostream &os, const message<T> &msg) {
    os << "id:" << int(msg.header.id) << " size:" << msg.size();
    return os;
  }

  template <typename DataType>
  friend message<T> &operator<<(message<T> &msg, const DataType &data) {
    // assert if data can be pushed into the vector
    static_assert(std::is_standard_layout<DataType>::value,
                  "Data is too complex");
    size_t msg_body_size = msg.body.size();
    // resize the body with
    msg.body.resize(msg_body_size + sizeof(DataType));
    // physically copy the data into newly allocated vector space
    // TODO: This can be done by just applying push_back will test later.
    std::memcpy(msg.body.data() + msg_body_size, &data, sizeof(DataType));
    // update size in msg header
    msg.header.size = msg.size();

    // Return the message
    return msg;
  }
};
} // namespace network
} // namespace xpd54