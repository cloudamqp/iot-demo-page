#include "DhtSensor.h"
#include <DHT.h>

// Adafruit "DHT sensor library" required
static DHT dht(DHTPIN, DHTTYPE);
static bool inited = false;

void DhtSensor::begin() {
  dht.begin();
  inited = true;
  Serial.printf("[SENSOR] DHT init OK (pin=%d, type=%s)\n",
                DHTPIN,
#if DHTTYPE == DHT11
                "DHT11"
#elif DHTTYPE == DHT22
                "DHT22"
#else
                "DHT"
#endif
  );
}

bool DhtSensor::read(float &tempC, float &humidity) {
  if (!inited) return false;
  humidity = dht.readHumidity();
  tempC    = dht.readTemperature(); // Celsius
  if (isnan(tempC) || isnan(humidity)) return false;
  return true;
}
