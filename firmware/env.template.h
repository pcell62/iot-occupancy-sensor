// env.template.h
// Copy this file, rename it to "env.h", and fill in your credentials.
// The file "env.h" is ignored by git to keep your secrets safe.

#ifndef ENV_H
#define ENV_H

// --- 1. Wi-Fi Credentials ---
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

// --- 2. ThingSpeak Credentials ---
#define TS_API_KEY "YOUR_THINGSPEAK_API_KEY"

// --- 3. Adafruit IO (MQTT) Credentials ---
#define AIO_USERNAME    "YOUR_ADAFRUIT_USERNAME"
#define AIO_KEY         "YOUR_ADAFRUIT_AIO_KEY"

// --- 4. ntfy Notification Topic ---
#define NTFY_TOPIC "YOUR_SECRET_NTFY_TOPIC"

#endif
