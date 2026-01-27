#pragma once
#include <Arduino.h>

namespace Portal {
  const char* apSsid();
  static const unsigned long AP_WINDOW_MS = 60000UL; 
  static const unsigned long AP_EXTEND_MS = 180000UL; 

  void startAPPortalWindow();
  void stopAP();
  bool running();
  void handleOnce(); // handle one client if present
  void maybeStopPortal();
}
