#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
// 


// ==================== HARDWARE CONFIGURATION ====================
// ESP32 I2C pins (default for DOIT DevKit v1)
#define I2C_SDA 21
#define I2C_SCL 22

// ADS1115 address (default 0x48, change if ADDR pin is connected)
#define ADS1115_ADDRESS 0x48
 // No argument
// CT Sensor Configuration
#define CT_RATIO 15.0f          // 15A per 1V (SCT-013-015)
#define LSB_VOLTAGE 0.125f          // 0.125 mV per count (GAIN_ONE)
#define BIAS_ADC_OFFSET 12893.0f    // Your zero offset
#define BIAS_VOLTAGE 1.612f         // Your measured bias voltage
#define VREF_LM4040 2.5f       // LM4040 reference voltage
#define R1_VALUE 10000.0f       // LM4040 series resistor (1.2kΩ)
#define R2_VALUE 87000.0f      // Bias divider resistor (10kΩ)
#define R3_VALUE 153000.0f      // Bias divider resistor (20kΩ)

// Measurement Configuration
#define SAMPLES_PER_READING 2000  // For RMS calculation
#define GAIN_SWITCH_THRESHOLD 2.0f // Amps: switch gain below this
#define CT_TURNS 1            // Number of primary turns (set to 2-3 for <2A)

// Fill this with measured values using a reference meter
struct CalPoint {
  float adcCounts;
  float actualCurrent;
};

// Example: Add 5-10 calibration points across your range
// Format: {ADC counts, actual current in amps}
const CalPoint calibrationTable[] = {
    {12893.0f, 0.00f},
    {13085.0f, 0.26f},
    {13181.0f, 0.48f},
    {13325.0f, 0.75f},
    {13469.0f, 1.04f},
    {13861.0f, 1.75f},
    {14213.0f, 2.39f},
    {15037.0f, 3.90f},
    {15341.0f, 4.45f},
    {16477.0f, 6.65f},
    {16717.0f, 7.12f},
    {17325.0f, 8.25f},
    {18821.0f, 10.90f},
    {19461.0f, 11.93f},
    {20397.0f, 13.80f},
};
const uint8_t numCalPoints = sizeof(calibrationTable) / sizeof(calibrationTable[0]);

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
int myFunction(int, int);
void calibrateZeroOffset();
// float readCurrentWithAutoGain();
float readCurrent();
void switchGain(GainState newGain);
float getLsbVoltage();
float getEstimatedCurrent();  
float applyCalibration(float rawCurrent);


void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== Heat Pump Current Monitor v1.0 ===");
  pinMode(LED_BUILTIN, OUTPUT);
  // ads.begin(0x48); 
  int result = myFunction(2, 3);
    // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); 
  // Initialize ADS1115
  if (!ads.begin(ADS1115_ADDRESS, &Wire)) {
    Serial.println("ERROR: Failed to initialize ADS1115!");
    Serial.println("Check I2C wiring and ADDR pin connection");
    while (1) {
      delay(1000);
      ESP.restart();
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

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
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
  ads.readADC_SingleEnded(3);  // Dummy read to flush
  delay(10);
  float lsbVolts = 0.0001875; // LSB voltage for GAIN_ONE
  float sumSq = 0.0f;
  const int samples = 2000;
  float sumSquares = 0.0;
  int16_t minReading = 32767, maxReading = -32768;
  // Serial.printf("Current gain setting check...\n");
  for (int i = 0; i < samples; i++) {
      int16_t reading = ads.readADC_SingleEnded(3);
      if (reading < minReading) minReading = reading;
      if (reading > maxReading) maxReading = reading;
      float acComponent = reading - zeroOffsetADC;
      sumSquares += acComponent * acComponent;
  }
  float rms_counts = sqrt(sumSquares / samples);
  float rms_voltage = rms_counts * lsbVolts;
  Serial.printf("\t%.3f\t\t%d\t\t%.2f\n", 
    rms_voltage, (int)rms_counts, zeroOffsetADC);
  Serial.printf("Min: %d, Max: %d, Peak-to-Peak: %d counts\n", 
      minReading, maxReading, maxReading - minReading);
  delayMicroseconds(80); // ESP32 can sample faster
  return rms_voltage;
}

// float readCurrentWithAutoGain() {
//   float sumSq = 0.0f;
//   int validSamples = 0;
  
//   // Auto-select gain based on expected current
//   float expectedCurrent = getEstimatedCurrent(); // Quick preliminary reading
//   GainState targetGain = (expectedCurrent < GAIN_SWITCH_THRESHOLD) ? GAIN_LOW : GAIN_HIGH;
  
//   if (targetGain != currentGain) {
//     switchGain(targetGain);
//   }
  
//   // Take RMS samples
//   for (int i = 0; i < SAMPLES_PER_READING; i++) {
//     int16_t rawADC = ads.readADC_SingleEnded(0);
    
//     // Remove bias and convert to voltage
//     float voltage = (rawADC - zeroOffsetADC) * getLsbVoltage();
    
//     // Convert to current (A = V / (1V/5A))
//     float currentInstant = voltage * CT_RATIO;
    
//     // Square for RMS
//     sumSq += currentInstant * currentInstant;
//     validSamples++;
    
//     // Maintain ~10kHz sampling rate
//     delayMicroseconds(100);
//   }
  
//   return sqrt(sumSq / validSamples);
// }

// ==================== GAIN MANAGEMENT ====================
void switchGain(GainState newGain) {
  currentGain = newGain;
  
  if (newGain == GAIN_LOW) {
    ads.setGain(GAIN_FOUR); // ±0.256V, 7.8125 µV/LSB
    Serial.println("\nSwitching to HIGH GAIN (±0.256V) for low current");
  } else {
    ads.setGain(GAIN_ONE); // ±1.024V, 31.25 µV/LSB
    Serial.println("\nSwitching to NORMAL GAIN (±1.024V) for high current");
  }
  
  delay(50); // Allow PGA to settle
}

float getLsbVoltage() {
  return (currentGain == GAIN_LOW) ? 0.0078125e-3f : 0.03125e-3f;
}

float getEstimatedCurrent() {
  // Quick single-sample reading for gain selection
  int16_t raw = ads.readADC_SingleEnded(0);
  float voltage = (raw - zeroOffsetADC) * getLsbVoltage();
  return abs(voltage) * CT_RATIO;
}

float applyCalibration(float rawCurrent) {
  // Multi-point linear interpolation
  if (numCalPoints < 2) return rawCurrent;
  
  // Find the bracket
  for (int i = 1; i < numCalPoints; i++) {
    if (rawCurrent <= calibrationTable[i].actualCurrent) {
      float x1 = calibrationTable[i-1].adcCounts;
      float y1 = calibrationTable[i-1].actualCurrent;
      float x2 = calibrationTable[i].adcCounts;
      float y2 = calibrationTable[i].actualCurrent;
      
      // Linear interpolation
      if (x2 - x1 != 0) {
        return y1 + (rawCurrent - x1) * (y2 - y1) / (x2 - x1);
      } else {
        return y1;
      }
    }
  }
  
  // Clamp to highest point
  return calibrationTable[numCalPoints-1].actualCurrent;
}