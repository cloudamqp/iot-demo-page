#pragma once
#include <Arduino.h>

// Change this if you wire the DHT11 to another pin
#ifndef DHTPIN
#define DHTPIN 19   
#endif

#ifndef DHTTYPE
#define DHTTYPE DHT11
#endif

namespace DhtSensor {
  void begin();
  bool read(float &tempC, float &humidity);
}
