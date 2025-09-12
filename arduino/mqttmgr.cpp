#include <WiFiS3.h>
#include <PubSubClient.h>
#include "ConfigStore.h"
#include "MqttMgr.h"
#if 0
#include <WiFiSSLClient.h>
static WiFiSSLClient _netSSL;
#endif

static WiFiClient _net; // plain client (1883)
static PubSubClient _client(_net);

static void _applyServerFromConfig() {
  Config cfg{};
  if (ConfigStore::load(cfg) && cfg.mhost[0]) {
    uint16_t port = cfg.mport ? cfg.mport : (MqttMgr::USE_TLS ? 8883 : 1883);
    _client.setServer(cfg.mhost, port);
  }
}
void MqttMgr::ensureConnected() {
  if (_client.connected()) return;


  _applyServerFromConfig();
  Config cfg{}; if (!ConfigStore::load(cfg) || !cfg.mhost[0]) { Serial.println("[MQTT] not configured"); return; }


  const char* u = (cfg.muser[0] ? cfg.muser : "");
  const char* p = (cfg.mpass[0] ? cfg.mpass : "");


  uint8_t retries = 3;
  while (!_client.connected() && retries--) {
    Serial.print("[MQTT] Connecting to "); Serial.print(cfg.mhost); Serial.print(":"); Serial.print(cfg.mport);
    Serial.print(" … ");
    if (_client.connect("UnoR4Client", u, p)) {
      Serial.println("connected");
    } 
    else {
      Serial.print("failed, rc="); Serial.println(_client.state());
      delay(2000);
    }
  }
}
void MqttMgr::loop() { _client.loop(); }
bool MqttMgr::publish(const char* topic, const char* payload, bool retained) {
  return _client.publish(topic, payload, retained);
}