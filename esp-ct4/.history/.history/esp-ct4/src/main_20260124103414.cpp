#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "CONFIG.h"
#include "ct.h"

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






