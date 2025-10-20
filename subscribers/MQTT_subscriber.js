// Import the MQTT client library for Node.js
const mqtt = require('mqtt')

// MQTT broker connection details (LavinMQ)
const BROKER = '<broker-name>'
const USERNAME = '<username>'
const PASSWORD = '<password>'
const PORT = 1883

// Topic names for the sensor data
// Topics determine how messages are routed through the broker
// Subscribers can subscribe to exact matches or use wildcards:
// - 'lavinmq/home/+' would match both temperature and humidity
// - 'lavinmq/home/#' would match all topics under lavinmq/home/
const TOPICT = 'lavinmq/home/temperature'
const TOPICH = 'lavinmq/home/humidity'

// Generate a unique client ID to avoid conflicts with other MQTT clients
const CLIENT_ID = 'terminal-consumer-' + Math.random().toString(16).slice(2)

// Create and configure MQTT client connection to the broker
const client = mqtt.connect({
  host: BROKER,
  port: PORT,
  protocol: PORT === 8883 ? 'mqtts' : 'mqtt',
  username: USERNAME,
  password: PASSWORD,
  clientId: CLIENT_ID,
  clean: true,
  keepalive: 60,
  reconnectPeriod: 1000
})

// Handle successful connection and subscribe to sensor topics
client.on('connect', () => {
  console.log('Connected to broker.')

  // Subscribe to the temperature and humidity topics with QoS 1 (at-least-once delivery)
  client.subscribe(
    { [TOPICT]: { qos: 1 }, [TOPICH]: { qos: 1 } },
    (err, granted) => {
      if (err) {
        console.error('Subscribe error:', err.message)
      } else {
        console.log(
          'Subscribed:',
          granted.map(g => `${g.topic} (QoS ${g.qos})`).join(', ')
        )
      }
    }
  )
})

// Handle incoming sensor data messages
client.on('message', (topic, payload) => {
  const text = payload.toString('utf8')
  const ts = new Date().toISOString()
  console.log(`[${ts}] ${topic}: ${text}`)
})

client.on('reconnect', () => console.log('Reconnecting...'))
client.on('close', () => console.log('Connection closed.'))
client.on('error', (err) => console.error('MQTT error:', err.message))

// Handle graceful shutdown
process.on('SIGINT', () => {
  console.log('\nDisconnecting...')
  client.end(true, () => process.exit(0))
})
