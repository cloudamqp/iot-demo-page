#include <Arduino.h>
#include <WiFiS3.h>
#include "ConfigStore.h"
#include "WiFiMgr.h"

bool WiFiMgr::connectWithTimeout(const char* ssid, const char* pass, unsigned long ms) {
  Serial.print("[WiFi] Connecting to '");
  Serial.print(ssid);
  Serial.print("' ... ");
  WiFi.begin(ssid, pass);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < ms) {
    delay(300);
    Serial.print('.');
  }
Serial.println();
if (WiFi.status() == WL_CONNECTED) {
  Serial.print("[WiFi] Connected. IP: ");
  Serial.println(WiFi.localIP());
  return true;
}
Serial.println("[WiFi] Connect failed.");
return false;
}


void WiFiMgr::ensureWifiFromStored() {
  if (WiFi.status() == WL_CONNECTED) return;
  Config stored{};
  if (ConfigStore::load(stored) && stored.ssid[0]) {
    connectWithTimeout(stored.ssid, stored.wpass, 15000);
  }
  else {
    Serial.println("[WiFi] No Wi-Fi saved. Power-cycle to reopen the portal.");
}
}