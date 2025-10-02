require 'mqtt'
require 'securerandom'
require 'time'

# Broker details
BROKER   = "<broker-name>"
PORT     = 1883
USERNAME = "<username>"
PASSWORD = "<password>"

# Topics
TOPICS   = {
  "lavinmq/home/temperature" => 1,
  "lavinmq/home/humidity"    => 1
}


CLIENT_ID     = "terminal-consumer-#{SecureRandom.hex(4)}" # random client Id
KEEPALIVE_SEC = 60
CLEAN_SESSION = true
USE_TLS       = (PORT == 8883)

# client connection to broker
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
