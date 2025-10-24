/*
 * FINAL CODE
 * Occupancy sensor with secure credentials management.
 */

// --- Wi-Fi & HTTP Libraries ---
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// --- MQTT Libraries ---
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// --- 1. LOAD SECRET CREDENTIALS ---
// This file is in .gitignore and is not public.
#include "env.h"

// --- 2. Assign Credentials ---
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;
String myApiKey = TS_API_KEY;

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    AIO_USERNAME
#define AIO_KEY         AIO_KEY

const char* ntfyTopic = NTFY_TOPIC;

// --- Pin Definitions ---
const int pirPin = D5;
const int ledPin = D1;

// --- State & Timer Variables ---
int ledState = LOW;
unsigned long lastMotionTime = 0;
const long occupancyTimeout = 10000; // 3 minutes

// --- Debounce & Cooldown Variables ---
int lastPirState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long wifiCooldownUntil = 0;

// --- ThingSpeak Logger Variables ---
long lastSendTime = 0;
int thingspeakInterval = 20000;
int lastSentLedState = LOW;

// --- Global Clients ---
WiFiClient client; // Used for both HTTP and MQTT
HTTPClient http;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish occupancyFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/occupancy");


void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(ledPin, LOW); 
  
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Sensor calibrated. System active.");
  Serial.printf("Occupancy timeout is set to %ld seconds.\n", occupancyTimeout / 1000);
  Serial.printf("ntfy topic: %s\n", ntfyTopic);
}

void loop() {
  unsigned long now = millis();

  // --- MQTT Connection Management ---
  MQTT_connect();

  // --- 1. MOTION DETECTION LOGIC (Debounced) ---
  if (now > wifiCooldownUntil) {
    int currentPirReading = digitalRead(pirPin);
    if (currentPirReading != lastPirState) {
      lastDebounceTime = now;
    }
  
    if ((now - lastDebounceTime) > debounceDelay) {
      // We now have a stable, debounced reading
      
      if (currentPirReading == HIGH) {
        lastMotionTime = now; 
        
        // --- NEW LOGIC: If motion is first detected ---
        if (ledState == LOW) {
          ledState = HIGH;
          digitalWrite(ledPin, HIGH);
          Serial.println("Motion DETECTED -> LED ON, Timer Started/Reset");

          // --- CONSOLIDATED WI-FI ACTIONS ---
          // 1. Set the cooldown FIRST
          wifiCooldownUntil = now + 5000; 
          Serial.println("--- Wi-Fi Active: Publishing... Sensor ignored for 5 sec ---");
          
          // 2. Publish to Adafruit IO dashboard
          publishToMQTT(ledState); 
          
          // 3. NEW: Send instant ntfy notification
          sendNtfyNotification("Motion detected in the room!");
        }
      }
    }
    lastPirState = currentPirReading;
  }
  
  // --- 2. TIMEOUT LOGIC ---
  if (ledState == HIGH && (now - lastMotionTime > occupancyTimeout)) {
    ledState = LOW;
    digitalWrite(ledPin, LOW);
    Serial.println("Timer EXPIRED -> LED OFF");
    
    // --- CONSOLIDATED WI-FI ACTIONS ---
    wifiCooldownUntil = now + 5000;
    Serial.println("--- Wi-Fi Active: Publishing... Sensor ignored for 5 sec ---");
    publishToMQTT(ledState); // Publish the "OFF" state
    // We don't send an ntfy notification when it turns off (but you could add one)
  }
  
  // --- 3. ThingSpeak Logger (Runs independently) ---
  if (now - lastSendTime > thingspeakInterval && ledState != lastSentLedState) {
    wifiCooldownUntil = now + 5000; 
    Serial.println("--- Preparing to send to ThingSpeak (Logger)... Sensor ignored for 5 sec ---");
    sendToThingSpeak(ledState);
    lastSentLedState = ledState;
    lastSendTime = now;
    Serial.println("--- ThingSpeak send complete. ---");
  }
}

// --- NEW: Function to send ntfy notification ---
void sendNtfyNotification(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    String url = "http://ntfy.sh/"; // Note: http, not https
    url += ntfyTopic;
    
    Serial.print("Sending ntfy notification... ");
    
    // Use the same 'client' and 'http' objects
    http.begin(client, url); 
    http.addHeader("Content-Type", "text/plain");
    
    // Send the message as an HTTP POST
    int httpCode = http.POST(message); 
    
    if (httpCode == 200) {
      Serial.println("Sent!");
    } else {
      Serial.printf("Failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected. Cannot send ntfy.");
  }
}

// --- UPDATED: MQTT function (cooldown line removed) ---
void publishToMQTT(int state) {
  int occupancyValue = (state == HIGH) ? 1 : 0;
  
  if (mqtt.connected()) {
    // The wifiCooldownUntil line was REMOVED from here
    Serial.print("Sending to Adafruit IO (MQTT)... ");
    if (occupancyFeed.publish(occupancyValue)) {
      Serial.println("Published!");
    } else {
      Serial.println("Failed to publish.");
    }
  } else {
    Serial.println("MQTT not connected. Skipping publish.");
  }
}

// --- Function to send data to ThingSpeak (HTTP) ---
void sendToThingSpeak(int state) {
  int occupancyValue = (state == HIGH) ? 1 : 0;
  
  if (WiFi.status() == WL_CONNECTED) {
    String url = "http://api.thingspeak.com/update?api_key=";
    url += myApiKey;
    url += "&field1=";
    url += String(occupancyValue);
    
    Serial.print("Sending to ThingSpeak (HTTP)... ");
    http.begin(client, url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      Serial.printf("Sent value %d, HTTP response code: %d\n", occupancyValue, httpCode);
    } else {
      Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected. Cannot send data.");
  }
}

// --- Function to manage MQTT connection ---
void MQTT_connect() {
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  int8_t ret;
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);
       retries--;
       if (retries == 0) {
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
