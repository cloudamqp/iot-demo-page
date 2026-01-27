#pragma once
#include <Arduino.h>

namespace WiFiMgr {
  bool connectWithTimeout(const char* ssid, const char* pass, unsigned long ms);
  void ensureWifiFromStored();
}
