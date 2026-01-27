#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "ConfigStore.h"
#include "MqttMgr.h"

static WiFiClient        _net_plain;
static WiFiClientSecure  _net_tls;
static Client*           _active_net = &_net_plain;
static PubSubClient      _client(_net_plain);
static uint16_t          _currentPort = 1883;

static void _applyServerFromConfig() {
  Config cfg{};
  if (ConfigStore::load(cfg) && cfg.mhost[0]) {
    uint16_t port = cfg.mport ? cfg.mport : (MqttMgr::USE_TLS ? 8883 : 1883);
    if(port == 8883){
      _net_tls.setInsecure();
      _active_net = &_net_tls;
    }
    else{
      _active_net = &_net_plain;
    }
    _client.setClient(*_active_net);
    _client.setServer(cfg.mhost, port);
    _client.setKeepAlive(25);
    _client.setSocketTimeout(10);
    _client.setBufferSize(512);

    _currentPort = port;
  }
}

void MqttMgr::ensureConnected() {
  if (_client.connected()) return;

  _applyServerFromConfig();
  Config cfg{};
  if (!ConfigStore::load(cfg) || !cfg.mhost[0]) {
    Serial.println("[MQTT] not configured");
    return;
  }

  const char* u = (cfg.muser[0] ? cfg.muser : "");
  const char* p = (cfg.mpass[0] ? cfg.mpass : "");

  const char* willTopic   = "lavinmq/home/status";
  const char* willMessage = "offline";
  const bool  willRetain  = true;
  const int   willQos     = 0;

  uint8_t retries = 3;
  while (!_client.connected() && retries--) {
    Serial.printf("[MQTT] Connecting to %s:%u … ", cfg.mhost, _currentPort);
    if (_client.connect("BeetleESP32C6", u, p, willTopic, willQos, willRetain, willMessage)) {
      Serial.println("connected");
      _client.publish("lavinmq/home/status", "online", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(_client.state());
      Serial.println(" (see https://pubsubclient.knolleary.net/api#state for codes)");
      delay(2000);
    }
  }
}

void MqttMgr::loop() {
  _client.loop();
}

bool MqttMgr::publish(const char* topic, const char* payload, bool retained) {
  bool ok = _client.publish(topic, payload, retained);
  if (!ok) {
    Serial.print("[MQTT] publish FAILED to '");
    Serial.print(topic);
    Serial.print("' (len=");
    Serial.print(strlen(payload));
    Serial.print(") state=");
    Serial.println(_client.state());
  } else {
    Serial.print("[MQTT] publish OK -> ");
    Serial.print(topic);
    Serial.print(" : ");
    Serial.println(payload);
  }
  return ok;
}
