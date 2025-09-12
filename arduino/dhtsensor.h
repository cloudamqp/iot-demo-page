#pragma once
#include <Arduino.h>
namespace DhtSensor {
  static const uint8_t DHT_PIN = 2;
  void begin();
  bool read(float &tempC, float &humidity);
}
