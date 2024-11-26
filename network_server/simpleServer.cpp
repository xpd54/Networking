#include <cstdint>
#include <iostream>
#include <memory>

#include "network_message.h"
#include "network_server.h"
#include "xpd54_network.h"

enum class CustomMsgTypes : uint32_t {
  ServerAccept,
  ServerDeny,
  ServerPing,
  MessageAll,
  ServerMessage,
};

class CustomServer : public xpd54::network::Network_Server<CustomMsgTypes> {
public:
  CustomServer(uint16_t nPort) : xpd54::network::Network_Server<CustomMsgTypes>(nPort) {}

protected:
  virtual bool on_client_connect(std::shared_ptr<xpd54::network::Connection<CustomMsgTypes>> client) override {
    xpd54::network::Message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ServerAccept;
    client->send(msg);
    return true;
  }

  virtual void on_client_disconnect(std::shared_ptr<xpd54::network::Connection<CustomMsgTypes>> client) override {
    std::cout << "Remove client [" << client->get_id() << "]" << '\n';
  }

  virtual void on_message(std::shared_ptr<xpd54::network::Connection<CustomMsgTypes>> client,
                          xpd54::network::Message<CustomMsgTypes> &msg) override {
    switch (msg.header.id) {
    case CustomMsgTypes::ServerPing: {
      std::cout << "[" << client->get_id() << "]"
                << ": Server Ping" << '\n';
    } break;
    case CustomMsgTypes::MessageAll: {
      std::cout << "[" << client->get_id() << "]"
                << ": Message All" << '\n';

      // custruct new message and send it to all client
      xpd54::network::Message<CustomMsgTypes> msg;
      msg.header.id = CustomMsgTypes::ServerMessage;
      msg << client->get_id();
      message_all_client(msg);
    } break;
    default:
      break;
    }
  }
};

int main() {
  CustomServer server(60000);
  server.start();
  while (1) {
    server.update(-1, true);
  }
  return 0;
}