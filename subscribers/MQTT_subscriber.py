import paho.mqtt.client as mqtt

# broker details
BROKER = "<broker-name>"
PORT = 1883
USERNAME = "<username>"
PASSWORD = "<password>"

# Topics
TOPICT = "lavinmq/home/temperature"
TOPICH = "lavinmq/home/humidity"

def on_connect(client, userdata, flags, reason_code, properties=None):
    if reason_code == 0:
        print("Connected to broker.")
        client.subscribe([(TOPICT, 1), (TOPICH, 1)])
        print(f"Subscribed to: {TOPICT}, {TOPICH}")
    else:
        print(f"Connect failed. reason_code={reason_code}")

def on_message(client, userdata, msg):
    print(f"[{msg.topic}] {msg.payload.decode('utf-8', errors='replace')}")

def on_disconnect(client, userdata, disconnect_flags, reason_code, properties=None):
    print(f"Disconnected. reason_code={reason_code}, flags={disconnect_flags}")

client = mqtt.Client(
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    protocol=mqtt.MQTTv311
)

client.username_pw_set(USERNAME, PASSWORD)
client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect


client.reconnect_delay_set(min_delay=1, max_delay=30)
client.connect(BROKER, PORT, keepalive=60)
client.loop_forever()
