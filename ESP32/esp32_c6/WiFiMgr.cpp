#include <Arduino.h>
#include <WiFi.h>
#include "ConfigStore.h"
#include "WiFiMgr.h"

bool WiFiMgr::connectWithTimeout(const char* ssid, const char* pass, unsigned long ms) {
  if (!ssid || !ssid[0]) return false;
  Serial.printf("[WiFi] Connecting to '%s' …\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, (pass && pass[0]) ? pass : nullptr);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < ms) {
    delay(300);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[WiFi] Connected. IP: "); Serial.println(WiFi.localIP());
    Serial.print("[WiFi] Gateway: "); Serial.println(WiFi.gatewayIP());
    Serial.print("[WiFi] DNS: ");     Serial.println(WiFi.dnsIP());
    return true;
  }
  Serial.printf("[WiFi] Connect failed (status=%d)\n", WiFi.status());
  return false;
}

void WiFiMgr::ensureWifiFromStored() {
  if (WiFi.status() == WL_CONNECTED) return;
  Config stored{};
  if (ConfigStore::load(stored) && stored.ssid[0]) {
    Serial.printf("[WiFi] Using stored SSID: %s\n", stored.ssid);
    connectWithTimeout(stored.ssid, stored.wpass, 15000);
  } else {
    Serial.println("[WiFi] No Wi-Fi saved. Power-cycle to reopen the portal.");
  }
}
