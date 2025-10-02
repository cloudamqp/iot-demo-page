#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "ConfigStore.h"
#include "Portal.h"
#include "WiFiMgr.h"
#include "MqttMgr.h"

// Lock I2C to the Beetle ESP32-C6 main I²C bus:
#define AHT20_SDA_PIN 19
#define AHT20_SCL_PIN 20
#include "DhtSensor.h"

// MQTT topics
#define TOPIC_BASE "lavinmq/home"
#define TOPIC_TEMP TOPIC_BASE "/temperature"
#define TOPIC_HUM  TOPIC_BASE "/humidity"

static unsigned long lastRead   = 0;
static const unsigned long READ_INTERVAL_MS = 1000; // 1s
static unsigned long lastTestPub = 0;

void setup() {
  Serial.begin(115200);
  delay(150);
  unsigned long t0 = millis(); while (!Serial && millis() - t0 < 1500) { delay(10); }

  Serial.println();
  Serial.println(F("[BOOT] Beetle ESP32-C6 starting…"));
  Serial.printf( "[BOOT] SDK: %s\n", ESP.getSdkVersion());
  Serial.printf( "[BOOT] Flash size: %u KB\n", ESP.getFlashChipSize() / 1024);

  ConfigStore::begin();
  WiFi.persistent(false);

  Serial.println(F("[SENSOR] Init AHT20/DHT20…"));
  DhtSensor::begin();

  Serial.println(F("[PORTAL] Opening SoftAP config window…"));
  Portal::startAPPortalWindow();
}

void loop() {
  if (Portal::running()) {
    Portal::handleOnce();
    Portal::maybeStopPortal();
    return; // portal has priority
  }

  // After portal closes, connect to stored Wi-Fi (non-blocking-ish)
  WiFiMgr::ensureWifiFromStored();

  if (WiFi.status() == WL_CONNECTED) {
    static bool announced = false;
    if (!announced) {
      announced = true;
      Serial.print("[WiFi] Connected. IP: ");
      Serial.println(WiFi.localIP());
    }
    // Only try MQTT once Wi-Fi is up
    MqttMgr::ensureConnected();
    MqttMgr::loop();

    // Heartbeat every 5s
    if (millis() - lastTestPub >= 5000) {
      lastTestPub = millis();
      char hb[64];
      snprintf(hb, sizeof(hb), "heartbeat %lu", (unsigned long)(millis()/1000));
      MqttMgr::publish(TOPIC_BASE "/heartbeat", hb, false);
    }

    // Sensor read/publish every second
    unsigned long now = millis();
    if (now - lastRead >= READ_INTERVAL_MS) {
      lastRead = now;

      float t, h;
      if (!DhtSensor::read(t, h)) {
        MqttMgr::publish(TOPIC_BASE "/sensor_error", "AHT20 read failed", false);
        Serial.println("[AHT20] read failed");
      } else {
        Serial.print(F("[SENSOR] T=")); Serial.print(t, 1);
        Serial.print(F(" °C  H=")); Serial.print(h, 1); Serial.println(F(" %"));

        String tmsg = String("Current Temperature: ") + String(t, 1);
        String hmsg = String("Current Humidity: ") + String(h, 2);

        bool ok1 = MqttMgr::publish(TOPIC_TEMP, tmsg.c_str(), false);
        bool ok2 = MqttMgr::publish(TOPIC_HUM,  hmsg.c_str(), false);
        Serial.println((ok1 && ok2) ? "[MQTT] publish OK" : "[MQTT] publish FAILED");
      }
    }
  }
}
