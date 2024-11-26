#include <chrono>
#include <cstdint>
#include <iostream>

#include "network_client.h"
#include "network_message.h"
#include "xpd54_network.h"
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
  uint32_t value;
  while (std::cin >> value) {
    if (value == 1) {
      client.message_all();
    } else {
      client.ping_server();
    }

    if (client.is_connected() && !client.incoming().empty()) {
      auto msg = client.incoming().pop_front().msg;
      switch (msg.header.id) {

      case CustomMsgTypes::ServerPing: {
        // Server has responded to a ping request
        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point timeThen;
        msg >> timeThen;
        std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
      } break;

      case CustomMsgTypes::ServerMessage: {
        // Server has responded to a ping request
        uint32_t client_id;
        msg >> client_id;
        std::cout << "Hello from [" << client_id << "]\n";
      } break;

      case CustomMsgTypes::ServerAccept:
        // Server has responded to a ping request
        std::cout << "Server Accepted Connection\n";
        break;
      default:
        break;
      }
    }
  }
  return 0;
}