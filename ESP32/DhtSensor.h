#pragma once
#include <Arduino.h>

namespace DhtSensor {
  void begin();
  bool read(float &tempC, float &humidity);
}
