# IoT Live Demo with LavinMQ

This project demonstrates using **LavinMQ** for an IoT live demo at conferences.
It contains two main parts:

- **`ESP32/`** → Arduino sketches (C/C++ code for the IoT device).
- **`landingpage/`** → A simple landing page built with Bootstrap, CSS, JavaScript, and HTML.



##  Project Structure

Iot-demo-page/
├── ESP32/ # Arduino sketches (C/C++ code for IoT device)
├── landingpage/ # Landing page (Bootstrap, CSS, JS, HTML)
├── subscribers # MQTT subscribers in different language
└── README.md # Project documentation

### consumers
- Python
- C#
- JS
- Ruby

### Hardware
- ESP32-c6 microcontroller
- DHT temperature & humidity sensor
- USB-C cable
- Jumper wires (red = power, black = ground, yellow = data)

### Software
- Arduino IDE
- Python 3.x
- Node js
- .Net
- ruby

##  Step 1: Setup the IoT Device

1. Connect the DHT sensor to the Arduino board with jumper wires:
   - **Red** → Power
   - **Black** → Ground
   - **Yellow** → Data

2. Connect the Arduino to your PC with a **USB-C cable**.

3. Open the **Arduino sketch** from the `ESP32/` folder in **Arduino IDE**.
   - Make sure to open and upload the **entire sketch folder** (not just a single `.ino` file).
   - Select the correct board from the  **Board** menu.
   - Upload the sketch to the board.

4. Open **Serial Monitor** from Arduino IDE.
   - Unplug and replug the USB-C cable.
   - The **IP address** of the board will appear.

---

## Step 2: Configure WiFi and MQTT

1. On your PC, connect to the IoT access point (**Demo_wifi**)

2. Open a browser and go to:  http://192.168.4.1


3. A mini portal will open:
- First, enter and save **MQTT details** (create a LavinMQ instance on [CloudAMQP](https://www.cloudamqp.com/)).
- Then go back, enter and save your **WiFi details**.

4. Close the portal and wait **1 minute** for the IoT device to connect to your WiFi.
- Once connected, `Demo_wifi` will disappear from available networks.
- The device will start publishing sensor data to LavinMQ.

👉 WiFi and MQTT credentials are stored on the board, so next time you just need to power it on.

---

## Step 3: Consume the Data

You can consume the messages in two ways:

### Option 1: Web Dashboard
1. Start a local HTTP server from the `landingpage/` folder:
```bash
cd landingpage
python3 -m http.server 8000
```
2. Open a browser and go to: http://localhost:8000/index.html

3. View temperature, humidity, and trend charts in the page

### Option 2: Terminal Subscriber

Go to the 'subscribers' directory and run one of the following based on your preferred language. Make sure that you replace the following variables in the files: `<broker-name>` `<username>` `<password>`.

#### Python
```bash
python3 MQTT_subscriber.py
```

#### C#
```bash
dotnet run
```

#### Node JS
```bash
node MQTT_subscriber.js
```

#### Ruby
```bash
ruby MQTT_subscriber.rb
```
This will print temperature and humidity readings directly in your terminal.
