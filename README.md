# Smart Occupancy-Based Appliance Control System using IoT

This is the official repository for the Course-Based Design Project for BCSE312L - Programming for IoT Boards.

## Abstract

**Problem:** Significant electrical energy is wasted when appliances are left running in unoccupied rooms.
**Objective:** To design and implement an IoT device that automatically controls appliances based on real-time room occupancy, provides a remote cloud dashboard, logs usage patterns, and sends instant mobile alerts.
**Outcome:** A functional ESP8266-based prototype that uses a PIR sensor and a smart software timer. It logs data to ThingSpeak (HTTP), publishes real-time status to Adafruit IO (MQTT), and sends push notifications via ntfy (HTTP).

---

## Hardware Requirements

* **Microcontroller:** ESP8266 (NodeMCU or Wemos D1 Mini)
* **Sensor:** HC-SR501 PIR Motion Sensor
* **Actuator:** 1x Standard LED (any color)
* **Resistor:** 1x 220Ω or 330Ω Resistor
* **Prototyping:** Solderless Breadboard
* **Wires:** 7x Female-to-Male Jumper Wires
* **Power:** Micro-USB Cable and 5V USB Power Supply

---

## Software & Cloud Services

1.  **Arduino IDE:** For compiling and uploading the firmware.
    * **ESP8266 Board Manager:** `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
    * **Required Libraries:**
        * `ESP8266WiFi`
        * `ESP8266HTTPClient`
        * `Adafruit MQTT Library`
2.  **ThingSpeak:** (Data Logging & Analytics)
    * Create a free account.
    * Create a new channel with one field (e.g., "Occupancy").
    * Get the **Write API Key**.
3.  **Adafruit IO:** (Real-Time Dashboard & MQTT)
    * Create a free account.
    * Create a new feed (e.g., "occupancy").
    * Get your **AIO Username** and **AIO Key**.
    * (Optional) Create a dashboard with an Indicator block linked to the "occupancy" feed.
4.  **ntfy.sh:** (Mobile Notifications)
    * Install the ntfy app on your mobile device.
    * Subscribe to a unique, secret topic (e.g., "priyanshu_room_6124").

---

## Installation and Configuration

1.  **Clone the Repository:**
    ```bash
    git clone [YOUR_GITHUB_REPO_URL]
    ```
2.  **Configure Firmware:**
    * In the `firmware/` folder, copy the `env.template.h` file and rename the copy to `env.h`.
    * Open the new `env.h` file and fill in your personal credentials (Wi-Fi, API Keys, etc.).
    * The `env.h` file is listed in `.gitignore` and will not be uploaded to GitHub.
3.  **Upload Firmware:**
    * Connect your ESP8266 board to your computer.
    * Select the correct Board (e.g., "NodeMCU 1.0") and Port in the Arduino IDE.
    * Click the **Upload** button.

---

## Usage Guidelines

1.  **Power On:** Power the device using a 5V USB power supply. The device will automatically connect to your Wi-Fi.
2.  **Calibration:** When first powered on, the PIR sensor requires 30-60 seconds to calibrate. Do not move in front of it during this time. The Serial Monitor will print "Sensor calibrated. System active." when ready.
3.  **Operation:**
    * When motion is detected, the LED will turn ON, and a notification will be sent to your ntfy app.
    * The device status will update to "1" on your Adafruit IO dashboard.
    * The device will ignore the sensor for 5 seconds (Wi-Fi cooldown) to prevent interference loops.
    * A 3-minute software timer starts. Any new motion will reset this timer.
    * If no motion is detected for 3 minutes, the LED will turn OFF, and the Adafruit IO dashboard will update to "0".
4.  **Monitoring:**
    * View the real-time status on your Adafruit IO dashboard.
    * View long-term occupancy patterns on your ThingSpeak channel.
    * Receive instant "Motion detected" alerts via the ntfy app.
