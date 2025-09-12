#include "DhtSensor.h"
#include "DHT.h"

static DHT _dht(DhtSensor::DHT_PIN, DHT11);
void DhtSensor::begin() { _dht.begin(); }

bool DhtSensor::read(float &tempC, float &humidity) {
  float t = _dht.readTemperature();
  float h = _dht.readHumidity();
  if (isnan(t) || isnan(h)) return false;
  tempC = t; humidity = h; return true;
}
