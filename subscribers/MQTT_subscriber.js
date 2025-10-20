const mqtt = require("mqtt");

// Broker details
const BROKER = "<broker-name>";
const PORT = 1883;
const USERNAME = "<username>";
const PASSWORD = "<password>";

// Topics
const TOPICT = "lavinmq/home/temperature";
const TOPICH = "lavinmq/home/humidity";

const CLIENT_ID = "terminal-consumer-" + Math.random().toString(16).slice(2); // client ID

// client connection to broker
const client = mqtt.connect({
  host: BROKER,
  port: PORT,
  protocol: PORT === 8883 ? "mqtts" : "mqtt",
  username: USERNAME,
  password: PASSWORD,
  clientId: CLIENT_ID,
  clean: true,
  keepalive: 60,
  reconnectPeriod: 1000,
});

// client subscription to the topics
client.on("connect", () => {
  console.log("Connected to broker.");
  client.subscribe(
    { [TOPICT]: { qos: 1 }, [TOPICH]: { qos: 1 } },
    (err, granted) => {
      if (err) {
        console.error("Subscribe error:", err.message);
      } else {
        console.log(
          "Subscribed:",
          granted.map(g => `${g.topic} (QoS ${g.qos})`).join(", ")
        );
      }
    }
  );
});

client.on("message", (topic, payload) => {
  const text = payload.toString("utf8");
  const ts = new Date().toISOString();
  console.log(`[${ts}] ${topic}: ${text}`);
});

client.on("reconnect", () => console.log("Reconnecting..."));
client.on("close", () => console.log("Connection closed."));
client.on("error", (err) => console.error("MQTT error:", err.message));

// Graceful exit
process.on("SIGINT", () => {
  console.log("\nDisconnecting...");
  client.end(true, () => process.exit(0));
});
