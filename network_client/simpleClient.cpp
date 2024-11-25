#include "network_message.h"
#include <cstdint>
#include <iostream>
#include <xpd54_network.h>
enum class CustomMsgTypes : uint32_t { FireBullet, MovePlayer };
int main() {
  // only allowd custom msg types
  xpd54::network::Message<CustomMsgTypes> msg;
  msg.header.id = CustomMsgTypes::FireBullet;
  int a = 1;
  bool b = true;
  float c = 3.14f;
  struct {
    float x;
    float y;
  } d[5];

  msg << a << b << c << d;
  a = 99;
  b = false;
  c = 99.34f;

  msg >> d >> c >> b >> a;

  return 0;
}