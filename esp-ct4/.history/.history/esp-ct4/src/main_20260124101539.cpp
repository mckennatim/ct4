#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "CONFIG.h"

          // Number of primary turns (set to 2-3 for <2A)

// ==================== GLOBAL OBJECTS ====================
Adafruit_ADS1115 ads;
bool adsInitialized = false;

// Calibration values (run calibrateZeroOffset() on startup)
float zeroOffsetADC = 0.0f;
float temperature = 25.0f;  // For temp compensation if sensor added

// Gain state management
enum GainState { GAIN_LOW, GAIN_HIGH };
GainState currentGain = GAIN_HIGH;

// put function declarations here:
void calibrateZeroOffset();
float readCurrent();
float getLsbVoltage();


void setup() {
  Serial.begin(115200);
  delay(2000); // Allow USB CDC to enumerate
  Serial.println("\n\n=== Heat Pump Current Monitor v1.0 ===");
  pinMode(LED_BUILTIN, OUTPUT);
  // ads.begin(0x48); 
    // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); 
  // Initialize ADS1115
  if (!ads.begin(ADS1115_ADDRESS, &Wire)) {
    Serial.println("ERROR: Failed to initialize ADS1115!");
    Serial.printf("Using Pins SDA: %d, SCL: %d\n", I2C_SDA, I2C_SCL);
    Serial.println("Check I2C wiring and ADDR pin connection");
    while (1) {
      delay(1000);
      // Removed auto-restart to prevent boot loop masking the error
      Serial.print(".");
    }
  }
  adsInitialized = true;
  ads.setGain(GAIN_ONE); // Start with ±1.024V range (16x for low current)
  currentGain = GAIN_HIGH;
  Serial.println("ADS1115 initialized successfully");
  // Calibrate zero offset (run with NO CURRENT through CT)
  Serial.println("Calibrating zero offset... ensure CT is not measuring any current");
  delay(3000);
  calibrateZeroOffset();
  Serial.println("\nSetup complete. Starting measurements...");
  Serial.println("Voltage[V]\tADC_Counts");
  Serial.println("-----------------------------------------------");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(700);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
    if (!adsInitialized) return;
  float current = readCurrent();
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



