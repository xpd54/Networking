#include <cstdint>

#include "network_client.h"
#include "network_connection.h"
#include "network_message.h"
#include "xpd54_network.h"
enum class CustomMsgTypes : uint32_t {
  ServerAccept,
  ServerDeny,
  ServerPing,
  MessageAll,
  ServerMessage,
};

class CustomClient : public xpd54::network::Network_Client<CustomMsgTypes> {};

int main() {
  CustomClient client;
  client.connect("127.0.0.1", 60000);
  return 0;
}