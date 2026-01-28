#pragma once

// --- Device Identity ---
// These match your specific request
#define DEV_ID          "CYURD006"
#define MQTT_SERVER     "sitebuilt.net"
#define MQTT_PORT       1884
#define MQTT_USER       "tim@sitebuilt.net"
#define MQTT_PASS       "geniot"

// --- Initial Connection Message ---
#define MSG_TIME_TOPIC   "time"
#define MSG_TIME_PAYLOAD "in mq.reconn->devid/time, <-/prg&/devtime"

// --- Feature Flags ---
// Comment out to disable
// #define USE_SENSORS 



// ----Sensor Configuration ----
// ----ct sensor
#define I2C_SDA 21  // Default for ESP32 (Original)
#define I2C_SCL 22