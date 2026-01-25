// WORKAROUND VERSION: Use GAIN_TWOTHIRDS permanently
// This accepts that the ADS1115 defaults to GAIN_TWOTHIRDS and doesn't fight it
// Trade-off: Slightly less resolution but measurements work correctly

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define ADS1115_ADDRESS 0x48

Adafruit_ADS1115 ads;
bool adsInitialized = false;
float zeroOffsetADC = 0.0f;

// LSB for GAIN_TWOTHIRDS (±6.144V range)
const float LSB_VOLTS = 0.0001875;  // V per count

void calibrateZeroOffset() {
  const int numSamples = 500;
  float sum = 0.0f;
  
  // Use GAIN_TWOTHIRDS
  ads.setGain(GAIN_TWOTHIRDS);
  delay(100);
  
  for (int i = 0; i < numSamples; i++) {
    sum += ads.readADC_SingleEnded(0);
    delay(10);
  }
  
  zeroOffsetADC = sum / numSamples;
  
  Serial.printf("Zero offset: %.2f counts (%.4fV)\n", 
                zeroOffsetADC, zeroOffsetADC * LSB_VOLTS);
}

float readCurrent() {
  // Don't change gain - just use TWOTHIRDS set in setup()
  
  const int samples = 2000;
  float sumSquares = 0.0;
  int16_t minReading = 32767, maxReading = -32768;
  
  for (int i = 0; i < samples; i++) {
    int16_t reading = ads.readADC_SingleEnded(0);
    if (reading < minReading) minReading = reading;
    if (reading > maxReading) maxReading = reading;
    float acComponent = reading - zeroOffsetADC;
    sumSquares += acComponent * acComponent;
  }
  
  float rms_counts = sqrt(sumSquares / samples);
  float rms_voltage = rms_counts * LSB_VOLTS;
  
  Serial.printf("RMS: %.3fV (%d counts) | Min: %d, Max: %d, P-P: %d\n", 
                rms_voltage, (int)rms_counts, 
                minReading, maxReading, maxReading - minReading);
  
  return rms_voltage;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ADS1115 Workaround - GAIN_TWOTHIRDS Only ===");
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // Use slower 100kHz for stability
  
  if (!ads.begin(ADS1115_ADDRESS, &Wire)) {
    Serial.println("ERROR: ADS1115 not found!");
    while (1) delay(1000);
  }
  
  // Set gain once and never change it
  ads.setGain(GAIN_TWOTHIRDS);
  delay(100);
  adsInitialized = true;
  
  Serial.println("Calibrating (no current)...");
  delay(2000);
  calibrateZeroOffset();
  
  Serial.println("\nReady. Starting measurements...\n");
}

void loop() {
  if (!adsInitialized) return;
  
  digitalWrite(LED_BUILTIN, HIGH);
  float voltage = readCurrent();
  delay(700);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
