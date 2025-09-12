#pragma once
#include <Arduino.h>

namespace MqttMgr{
  // Set USE_TLS to true if your broker requires TLS on 8883.
  static const bool USE_TLS = false;

  void ensureConnected();
  void loop();
  bool publish(const char* topic, const char* payload, bool retained);
}