#include "ct.h"


float readCurrent() {
  ads.setGain(GAIN_ONE);
  delay(100);  // Increase from 10ms to 100ms
  ads.readADC_SingleEnded(0);  // Dummy read to flush
  delay(10);
  float lsbVolts = 0.0001875; // LSB voltage for GAIN_ONE
  float sumSq = 0.0f;
  const int samples = 2000;
  float sumSquares = 0.0;
  int16_t minReading = 32767, maxReading = -32768;
  // Serial.printf("Current gain setting check...\n");
  for (int i = 0; i < samples; i++) {
      int16_t reading = ads.readADC_SingleEnded(0);
      if (reading < minReading) minReading = reading;
      if (reading > maxReading) maxReading = reading;
      float acComponent = reading - zeroOffsetADC;
      sumSquares += acComponent * acComponent;
  }
  float rms_counts = sqrt(sumSquares / samples);
  float rms_voltage = rms_counts * lsbVolts;
  float m = 104.7;
  float b = 0.9221;
  float rms_current = (rms_voltage*1000 - b) / m;
  Serial.printf("\t%.3f\t\t%d\t\t%.2f\n", 
    rms_voltage, (int)rms_counts, rms_current);
  Serial.printf("Min: %d, Max: %d, Peak-to-Peak: %d counts\n", 
      minReading, maxReading, maxReading - minReading);
  delayMicroseconds(80); // ESP32 can sample faster
  return rms_current;
}

// ==================== CALIBRATION ====================
void calibrateZeroOffset() {
  const int numSamples = 500;
  float sum = 0.0f;
  // Ensure CT is NOT measuring current during calibration
  ads.setGain(GAIN_ONE); // Use most sensitive range for offset
  delay(50);
  for (int i = 0; i < numSamples; i++) {
    sum += ads.readADC_SingleEnded(0);
    delay(10);
  }
  zeroOffsetADC = sum / numSamples;
  Serial.printf("Zero offset calibrated: %.2f ADC counts\n", zeroOffsetADC);
  Serial.printf("This corresponds to: %.3f V bias\n", zeroOffsetADC * 0.000125);
  // Restore gain setting
  ads.setGain(GAIN_ONE);
  currentGain = GAIN_HIGH;
} 
