#include "../lib/include/network_client.h"
#include "../lib/include/network_message.h"
#include "../lib/include/xpd54_network.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <future>
#include <iostream>
#include <thread>
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
  int count = 0;
  for (int i = 0; i < 200; ++i) {
    if (i % 3 == 1) {
      count++;
      client.message_all();
    } else {
      client.ping_server();
    }
  }
  std::cout << count << '\n';
  std::this_thread::sleep_for(std::chrono::milliseconds(250));
  return 0;
}