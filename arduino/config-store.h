#pragma once
#include <Arduino.h>
#include <EEPROM.h>


// Maximum lengths (without null terminator)
#define MAX_SSID_LEN 32
#define MAX_WPASS_LEN 63
#define MAX_MQTT_HOST 64
#define MAX_MQTT_USER 64
#define MAX_MQTT_PASS 64


struct Config {
  // Wi‑Fi
  char ssid[MAX_SSID_LEN + 1];
  char wpass[MAX_WPASS_LEN + 1];


  // MQTT
  char mhost[MAX_MQTT_HOST + 1];
  uint16_t mport;
  char muser[MAX_MQTT_USER + 1];
  char mpass[MAX_MQTT_PASS + 1];


  uint32_t crc; // set over the struct with crc=0
};


namespace ConfigStore {
  // Start address in emulated EEPROM
  static const int EEPROM_ADDR = 0;


  bool load(Config &cfg);
  void save(const Config &cfg);
  void clearAll();
  void saveWifiOnly(const char* ssid, const char* pass, const Config* prev = nullptr);
  void saveMqttOnly(const char* host, uint16_t port, const char* user, const char* pass, const Config* prev = nullptr);
  void clearWifi();
  void clearMqtt();
}