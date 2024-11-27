#include "../lib/include/network_client.h"
#include "../lib/include/network_message.h"
#include "../lib/include/xpd54_network.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
enum class CustomMsgTypes : uint32_t {
  ServerAccept,
  ServerDeny,
  ServerPing,
  MessageAll,
  ServerMessage,
};

class CustomClient : public xpd54::network::Network_Client<CustomMsgTypes> {
public:
  void ping_server() {
    xpd54::network::Message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ServerPing;
    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    msg << timeNow;
    send(msg);
  }

  void message_all() {
    xpd54::network::Message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::MessageAll;
    send(msg);
  }
};

int main() {
  CustomClient client;
  client.connect("127.0.0.1", 60000);
  int value;
  bool quite = false;
  while (!quite) {
    std::cin >> value;
    if (value == 1) {
      client.message_all();
    } else if (value == 2) {
      client.ping_server();
    } else {
      quite = true;
    }
    if (!client.is_connected()) {
      quite = true;
    }
  }
  return 0;
}