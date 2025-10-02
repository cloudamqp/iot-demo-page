#include "DhtSensor.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>

static Adafruit_AHTX0 aht;
static bool ahtOk = false;

void DhtSensor::begin() {
  #if defined(AHT20_SDA_PIN) && defined(AHT20_SCL_PIN)
    Wire.begin(AHT20_SDA_PIN, AHT20_SCL_PIN);
    Serial.printf("[I2C] Using SDA=%d SCL=%d\n", AHT20_SDA_PIN, AHT20_SCL_PIN);
  #else
    Wire.begin(); // default pins if not defined
    Serial.println("[I2C] Using default SDA/SCL");
  #endif

  ahtOk = aht.begin();
  Serial.println(ahtOk ? "[SENSOR] AHT20/DHT20 OK" : "[SENSOR] AHT20/DHT20 NOT FOUND");
  if (!ahtOk) { delay(50); ahtOk = aht.begin(); if (ahtOk) Serial.println("[SENSOR] AHT20/DHT20 OK on retry"); }
}

bool DhtSensor::read(float &tempC, float &humidity) {
  if (!ahtOk) return false;
  sensors_event_t hum, temp;
  aht.getEvent(&hum, &temp);
  if (isnan(temp.temperature) || isnan(hum.relative_humidity)) return false;
  tempC    = temp.temperature;
  humidity = hum.relative_humidity;
  return true;
}
