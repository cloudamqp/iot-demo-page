# Import Paho MQTT client library for Python
import paho.mqtt.client as mqtt

# MQTT broker connection details (LavinMQ)
BROKER = '<broker-name>'
USERNAME = '<username>'
PASSWORD = '<password>'
PORT = 1883

# Topic names for sensor data
# Topics determine how messages are routed through the broker
# Subscribers can subscribe to exact matches or use wildcards:
# - 'lavinmq/home/+' would match both temperature and humidity
# - 'lavinmq/home/#' would match all topics under lavinmq/home/
TOPICT = "lavinmq/home/temperature"
TOPICH = "lavinmq/home/humidity"

# Handle successful connection and subscribe to sensor topics
def on_connect(client, userdata, flags, reason_code, properties=None):
    if reason_code == 0:
        print("Connected to broker.")
        client.subscribe([(TOPICT, 1), (TOPICH, 1)])
        print(f"Subscribed to: {TOPICT}, {TOPICH}")
    else:
        print(f"Connect failed. reason_code={reason_code}")

# Handle incoming sensor data messages
def on_message(client, userdata, msg):
    print(f"[{msg.topic}] {msg.payload.decode('utf-8', errors='replace')}")

def on_disconnect(client, userdata, disconnect_flags, reason_code, properties=None):
    print(f"Disconnected. reason_code={reason_code}, flags={disconnect_flags}")

# Create and configure MQTT client
client = mqtt.Client(
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    protocol=mqtt.MQTTv311
)

client.username_pw_set(USERNAME, PASSWORD)
client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect

# Connect to broker and start listening for messages
client.reconnect_delay_set(min_delay=1, max_delay=30)
client.connect(BROKER, PORT, keepalive=60)
client.loop_forever()
