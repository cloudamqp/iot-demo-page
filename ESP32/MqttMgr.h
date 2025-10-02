#pragma once
#include <Arduino.h>

namespace MqttMgr {
  static const bool USE_TLS = false; // flip to true if you add WiFiClientSecure + root CA
  void ensureConnected();
  void loop();
  bool publish(const char* topic, const char* payload, bool retained);
}
