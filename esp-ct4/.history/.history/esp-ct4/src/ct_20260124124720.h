#ifndef ct_h
#define ct_h

#include <Arduino.h>

struct CT_Config {
  uint8_t   pin;        // ADS1115 Channel (0-3)
  adsGain_t gain;       // GAIN_ONE, GAIN_TWO_THIRDS
  float     lsbVolts;   // The voltage step for that gain (e.g., 0.000125)
  float     m;          // Slope (Calibration) - MUST be float
  float     b;          // Intercept (Calibration) - MUST be float
  int       capacity;   // Metadata (e.g., 30A, 100A)
  const char* name;     // Optional: Label for logging
};

void calibrateZeroOffset();
float readCurrent();
bool setupCT(); 

#endif