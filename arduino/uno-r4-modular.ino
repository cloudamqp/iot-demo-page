#include <Arduino.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include "ConfigStore.h"
#include "Portal.h"
#include "WiFiMgr.h"
#include "MqttMgr.h"
#include "DhtSensor.h"

// Topics
#define TOPIC_BASE "lavinmq/home"
#define TOPIC_TEMP TOPIC_BASE "/temperature"
#define TOPIC_HUM  TOPIC_BASE "/humidity"

static unsigned long lastRead = 0;
static const unsigned long READ_INTERVAL_MS = 1000; // 1 second

void setup() {
  Serial.begin(115200);
  delay(150);
  Serial.println("\n[PublishDemo] Booting…");

  DhtSensor::begin();
  Portal::startAPPortalWindow();
}

void loop() {
  if (Portal::running()) {
    Portal::handleOnce();
    Portal::maybeStopPortal();
    return; // do nothing else while portal is open
  }

  // After the portal window closes, connect to your chosen Wi-Fi (from EEPROM)
  WiFiMgr::ensureWifiFromStored();

  if (WiFi.status() == WL_CONNECTED) {
    static bool announced = false;
    if (!announced) {
      announced = true;
      Serial.print("[WiFiOnly] Connected. IP: ");
      Serial.println(WiFi.localIP());
      Serial.println("You can now ping this IP or open a simple test server here if you want.");
    }
  }

  // Ensure MQTT connection using stored settings
  MqttMgr::ensureConnected();
  MqttMgr::loop();

  unsigned long now = millis();
  if (now - lastRead >= READ_INTERVAL_MS) {
    lastRead = now;

    float t, h;
    if (!DhtSensor::read(t, h)) {
      Serial.println("[DHT] read failed");
      return;
    }

    // Build messages and publish every second
    String tmsg = String("Current Temperature: ") + String(t, 1);
    String hmsg = String("Current Humidity: ") + String(h, 2);

    bool ok1 = MqttMgr::publish(TOPIC_TEMP, tmsg.c_str(), false);
    bool ok2 = MqttMgr::publish(TOPIC_HUM,  hmsg.c_str(), false);

    if (ok1 && ok2) {
      Serial.println("[MQTT] publish OK");
    } else {
      Serial.println("[MQTT] publish FAILED");
    }
  }
}
