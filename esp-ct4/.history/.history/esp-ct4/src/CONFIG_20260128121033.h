#pragma once

// --- Device Identity ---
// These match your specific request
#define DEV_ID          "DYURD001"
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
// struct CT_Config {
//   uint8_t   pin;        // ADS1115 Channel (0-3)
//   adsGain_t gain;       // GAIN_ONE, GAIN_TWO_THIRDS
//   float     lsbVolts;   // The voltage step for that gain (e.g., 0.000125)
//   float     m;          // Slope (Calibration) - MUST be float
//   float     b;          // Intercept (Calibration) - MUST be float
//   int       capacity;   // Metadata (e.g., 30A, 100A)
//   float     threshold;
//   const char* name;     // Optional: Label for logging
// };