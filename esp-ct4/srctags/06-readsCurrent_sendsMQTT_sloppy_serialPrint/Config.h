#pragma once

// --- Device Identity ---
// These match your specific request
#define DEV_ID          "CYURD127"
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
#define USE_CT_SENSORS  // <--- Enable CT Sensor Block
#ifdef USE_CT_SENSORS
#include <Adafruit_ADS1X15.h>

#define I2C_SDA 21  // Default for ESP32 (Original)
#define I2C_SCL 22

struct CT_Config {
  uint8_t   pin;        // ADS1115 Channel (0-3)
  adsGain_t gain;       // GAIN_ONE, GAIN_TWO_THIRDS
  float     lsbVolts;   // The voltage step for that gain (e.g., 0.000125)
  float     m;          // Slope (Calibration) - MUST be float
  float     b;          // Intercept (Calibration) - MUST be float
  int       capacity;   // Metadata (e.g., 30A, 100A)
  float     threshold;
  const char* name;     // Optional: Label for logging
};

// static: Prevents "multiple definition" errors when included in multiple files (Internal Linkage)
static const CT_Config ct_sensors[4] = {
  {0, GAIN_ONE, 0.0001875,  104.7, 0.922, 15, .3, "Heat_Pump"},
  {1, GAIN_ONE, 0.0001875,  104.7, 0.922, 15, .3, "Solar"},
  {2, GAIN_ONE, 0.0001875,  104.7, 0.922, 50, .3, "EV"},
  {3, GAIN_ONE, 0.0001875,  104.7, 0.922, 15, .3, "empty"},
};
#endif // USE_CT_SENSORS
