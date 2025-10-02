#include "ConfigStore.h"

static uint32_t crc32_buf(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFF;
  while (len--) {
    crc ^= *data++;
    for (int i = 0; i < 8; i++) {
      crc = (crc >> 1) ^ (0xEDB88320 & (~(crc & 1) + 1));
    }
  }
  return ~crc;
}

void ConfigStore::begin() {
  const size_t eepromSize = 512;
  EEPROM.begin(eepromSize);
}

bool ConfigStore::load(Config &cfg) {
  EEPROM.get(EEPROM_ADDR, cfg);
  Config tmp = cfg; tmp.crc = 0;
  bool empty = (cfg.ssid[0] == '\0' && cfg.mhost[0] == '\0');
  if (empty) return false;
  return cfg.crc == crc32_buf((uint8_t*)&tmp, sizeof(Config));
}

void ConfigStore::save(const Config &in) {
  Config c = in;
  c.crc = 0;
  c.crc = crc32_buf((uint8_t*)&c, sizeof(Config));
  EEPROM.put(EEPROM_ADDR, c);
  EEPROM.commit();
}

void ConfigStore::clearAll() {
  Config c{}; EEPROM.put(EEPROM_ADDR, c); EEPROM.commit();
}

void ConfigStore::saveWifiOnly(const char* ssid, const char* pass, const Config* prev) {
  Config c{}; if (prev) c = *prev;
  if (ssid) { strncpy(c.ssid, ssid, MAX_SSID_LEN); c.ssid[MAX_SSID_LEN] = '\0'; }
  if (pass) { strncpy(c.wpass, pass, MAX_WPASS_LEN); c.wpass[MAX_WPASS_LEN] = '\0'; }
  if (c.mport == 0) c.mport = 1883;
  save(c);
}

void ConfigStore::saveMqttOnly(const char* host, uint16_t port, const char* user, const char* pass, const Config* prev) {
  Config c{}; if (prev) c = *prev;
  if (host && host[0]) { strncpy(c.mhost, host, MAX_MQTT_HOST); c.mhost[MAX_MQTT_HOST] = '\0'; }
  if (port) c.mport = port; else if (!c.mport) c.mport = 1883;
  if (user) { strncpy(c.muser, user, MAX_MQTT_USER); c.muser[MAX_MQTT_USER] = '\0'; }
  if (pass && pass[0]) { strncpy(c.mpass, pass, MAX_MQTT_PASS); c.mpass[MAX_MQTT_PASS] = '\0'; }
  save(c);
}

void ConfigStore::clearWifi() {
  Config c{}; (void)load(c);
  c.ssid[0] = '\0';
  c.wpass[0] = '\0';
  save(c);
}

void ConfigStore::clearMqtt() {
  Config c{}; (void)load(c);
  c.mhost[0] = '\0';
  c.mport = 0;
  c.muser[0] = '\0';
  c.mpass[0] = '\0';
  save(c);
}
