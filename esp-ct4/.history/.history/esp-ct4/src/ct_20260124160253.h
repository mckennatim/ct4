#ifndef ct_h
#define ct_h

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

struct CT_Config {
  uint8_t   pin;        // ADS1115 Channel (0-3)
  adsGain_t gain;       // GAIN_ONE, GAIN_TWO_THIRDS
  float     lsbVolts;   // The voltage step for that gain (e.g., 0.000125)
  float     m;          // Slope (Calibration) - MUST be float
  float     b;          // Intercept (Calibration) - MUST be float
  int       capacity;   // Metadata (e.g., 30A, 100A)
  const char* name;     // Optional: Label for logging
};

struct CT_Runtime {
  float currentValue;       // The raw reading we just took
  float lastReportedValue;  // The value we successfully sent to MQTT last time
};

void calibrateZeroOffset();
float readCurrent();
bool setupCT(); 

#endif