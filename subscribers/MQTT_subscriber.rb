# Import Ruby MQTT library
require 'mqtt'
require 'securerandom'
require 'time'

# MQTT broker connection details (LavinMQ)
BROKER   = "<broker-name>"
USERNAME = "<username>"
PASSWORD = "<password>"
PORT     = 1883

# Topic names for sensor data with QoS levels
# Topics determine how messages are routed through the broker
# Subscribers can subscribe to exact matches or use wildcards:
# - 'lavinmq/home/+' would match both temperature and humidity
# - 'lavinmq/home/#' would match all topics under lavinmq/home/
TOPICS   = {
  "lavinmq/home/temperature" => 1,  # QoS 1 for temperature
  "lavinmq/home/humidity"    => 1   # QoS 1 for humidity
}

# Generate unique client ID and connection settings
CLIENT_ID     = "terminal-consumer-#{SecureRandom.hex(4)}"
KEEPALIVE_SEC = 60
CLEAN_SESSION = true
USE_TLS       = (PORT == 8883)

# Connect to broker and listen for sensor data
def connect_and_stream
  MQTT::Client.connect(
    host:         BROKER,
    port:         PORT,
    username:     USERNAME,
    password:     PASSWORD,
    client_id:    CLIENT_ID,
    keep_alive:   KEEPALIVE_SEC,
    clean_session: CLEAN_SESSION,
    ssl:          USE_TLS
  ) do |c|
    c.subscribe(TOPICS)
    puts "Connected. Subscribed to: #{TOPICS.keys.join(', ')}"

    # Handle incoming sensor messages
    c.get do |topic, message|
      ts = Time.now.utc.iso8601
      puts "[#{ts}] #{topic}: #{message}"
    end
  end
end

# Auto-reconnect
backoff = 1
loop do
  begin
    connect_and_stream
    backoff = 1
  rescue Interrupt
    puts "\nDisconnecting..."
    break
  rescue => e
    warn "Disconnected: #{e.class} - #{e.message}"
    sleep backoff
    backoff = [backoff * 2, 30].min
  end
end
