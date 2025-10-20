// Import required .NET MQTT libraries
using System;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Protocol;

class Program
{
    // MQTT broker connection details (LavinMQ)
    private const string BROKER = "<broker-name>";
    private const string USERNAME = "<username>";
    private const string PASSWORD = "<password>";
    private const int PORT = 1883;

    // Topic names for sensor data
    // Topics determine how messages are routed through the broker
    // Subscribers can subscribe to exact matches or use wildcards:
    // - 'lavinmq/home/+' would match both temperature and humidity
    // - 'lavinmq/home/#' would match all topics under lavinmq/home/
    private const string TOPICT = "lavinmq/home/temperature";
    private const string TOPICH = "lavinmq/home/humidity";


    static async Task Main()
    {
        // Create and configure MQTT client
        var factory = new MqttFactory();
        using var client = factory.CreateMqttClient();

        // Handle incoming sensor data messages
        client.ApplicationMessageReceivedAsync += e =>
        {
            var topic = e.ApplicationMessage.Topic ?? "";
            var seg = e.ApplicationMessage.PayloadSegment;
            string payload = seg.Array == null
                ? ""
                : Encoding.UTF8.GetString(seg.Array, seg.Offset, seg.Count);
            Console.WriteLine($"[{DateTime.UtcNow:O}] {topic}: {payload}");
            return Task.CompletedTask;
        };

        // Handle successful connection and subscribe to sensor topics
        client.ConnectedAsync += async e =>
        {
            Console.WriteLine("Connected to broker.");
            // Subscribe to both topics with QoS 1 (at-least-once delivery)
            await client.SubscribeAsync(new MqttClientSubscribeOptionsBuilder()
                .WithTopicFilter(f => f.WithTopic(TOPICT).WithQualityOfServiceLevel(MqttQualityOfServiceLevel.AtLeastOnce))
                .WithTopicFilter(f => f.WithTopic(TOPICH).WithQualityOfServiceLevel(MqttQualityOfServiceLevel.AtLeastOnce))
                .Build());
            Console.WriteLine($"Subscribed to: {TOPICT}, {TOPICH}");
        };

        client.DisconnectedAsync += async e =>
        {
            Console.WriteLine($"Disconnected: {e.Reason}");
            await Task.Delay(TimeSpan.FromSeconds(2));
            try { await client.ConnectAsync(BuildOptions()); }
            catch (Exception ex) { Console.WriteLine($"Reconnect failed: {ex.Message}"); }
        };

        try
        {
            await client.ConnectAsync(BuildOptions());
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Connect failed: {ex.Message}");
        }

        // loop_forever()
        await Task.Delay(Timeout.InfiniteTimeSpan);
    }

    private static MqttClientOptions BuildOptions()
    {
        var builder = new MqttClientOptionsBuilder()
            .WithClientId($"csharp-subscriber-{Guid.NewGuid():N}")
            .WithCredentials(USERNAME, PASSWORD)
            .WithCleanSession()
            .WithKeepAlivePeriod(TimeSpan.FromSeconds(60))
            .WithProtocolVersion(MQTTnet.Formatter.MqttProtocolVersion.V311);

        if (PORT == 8883)
            builder = builder.WithTcpServer(BROKER, PORT).WithTls();
        else
            builder = builder.WithTcpServer(BROKER, PORT);

        return builder.Build();
    }
}
