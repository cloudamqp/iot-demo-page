using System;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Protocol;

class Program
{
    // broker details
    private const string BROKER = "<broker-name>";
    private const int PORT = 1883;
     private const string USERNAME = "<username>";
    private const string PASSWORD = "<password>";

    // Topics
    private const string TOPICT = "lavinmq/home/temperature";
    private const string TOPICH = "lavinmq/home/humidity";


    static async Task Main()
    {
        var factory = new MqttFactory();
        using var client = factory.CreateMqttClient();

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

        client.ConnectedAsync += async e =>
        {
            Console.WriteLine("Connected to broker.");
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
