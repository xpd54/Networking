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

  virtual bool on_client_connect(std::shared_ptr<xpd54::network::Connection<CustomMsgTypes>> client) override {
    return true;
  }

  virtual void on_client_disconnect(std::shared_ptr<xpd54::network::Connection<CustomMsgTypes>> client) override {}

  virtual void on_message(std::shared_ptr<xpd54::network::Connection<CustomMsgTypes>> client,
                          xpd54::network::Message<CustomMsgTypes> &msg) override {}
};

int main() {
  CustomServer server(60000);
  server.start();
  while (1) {
    server.update();
  }
  return 0;
}