# IoT Live Demo with LavinMQ

This project demonstrates using **LavinMQ** for an IoT live demo at conferences.
It contains two main parts:

- **`esp32_c6/`** → Arduino sketches (C/C++ code for the IoT device). Needs to be uploaded to the ESP32 microcontroller.
- **`landingpage/`** → A simple landing page built with Bootstrap, CSS, JavaScript, and HTML.

## Project Structure

```
iot-demo-page/
├── esp32_c6/ # Arduino sketches (C/C++ code for IoT device)
├── landingpage/ # Landing page (Bootstrap, CSS, JS, HTML)
├── subscribers/ # MQTT subscribers in different languages
└── README.md # Project documentation
```

### Consumers
- Python
- C#
- JS
- Ruby

### Hardware
- ESP32-C6 microcontroller
- DHT temperature & humidity sensor
- USB-C cable
- Jumper wires (red = power, black = ground, yellow = data)

### Software
- Arduino IDE
- Python 3.x
- Node.js
- .NET
- Ruby

##  Step 1: Setup the IoT Device

1. Connect the DHT sensor to the Arduino board with jumper wires:
   - **Red** → Power
   - **Black** → Ground
   - **Yellow** → Data

2. Connect the Arduino to your PC with a **USB-C cable**.

3. Open the **Arduino sketch** from the `esp32_c6/` folder in **Arduino IDE**.
   - Make sure to open and upload the **entire sketch folder** (not just a single `.ino` file).
   - Ensure you have the following libraries installed:
      - DHT sensor library by Adafruit
      - PubSubClient by Nick O'Leary
   - Select the correct board from the **Board** menu.
   - Upload the sketch to the board.

4. Open **Serial Monitor** from Arduino IDE.
   - Unplug and replug the USB-C cable.
   - The **IP address** of the board will appear.

## Step 2: Configure Wi-Fi and MQTT

1. On your PC, connect to the IoT access point (`demowifi-<deviceid>`).

2. Open a browser and go to: http://192.168.4.1

3. A mini portal will open:

- Enter your MQTT details and press Save.
- Enter your Wi-Fi credentials and press Save.

4. Close the portal and wait **1 minute** for the IoT device to connect to your Wi-Fi.
- Once connected, `demowifi-<deviceid>` will disappear from available networks.
- The device will start publishing sensor data to LavinMQ.

👉 Wi-Fi and MQTT credentials are stored on the board, so next time you just need to power it on.

## Step 3: Consume the Data

You can consume the messages in two ways.

### Option 1: Terminal Subscriber

1. Navigate to the `subscribers/` directory.
2. Choose your preferred language and edit the corresponding file:
   - **JavaScript**: `MQTT_subscriber.js`
   - **Python**: `MQTT_subscriber.py`
   - **Ruby**: `MQTT_subscriber.rb`
   - **C#**: `Program.cs`
3. Replace the placeholder values with your LavinMQ instance credentials:
   - `<broker-name>` → Your LavinMQ broker URL
   - `<username>` → Your LavinMQ username
   - `<password>` → Your LavinMQ password
4. Run the appropriate command for your chosen language.

#### Python
```bash
python3 MQTT_subscriber.py
```

#### C#
```bash
dotnet run
```

#### Node.js
```bash
node MQTT_subscriber.js
```

#### Ruby
```bash
ruby MQTT_subscriber.rb
```
This will print temperature and humidity readings directly in your terminal.


### Option 2: Web Dashboard
1. Start a local HTTP server from the `landingpage/` folder:
```bash
cd landingpage
python3 -m http.server 8000
```
2. Open a browser and go to: http://localhost:8000/index.html

3. View temperature, humidity, and trend charts on the page.
