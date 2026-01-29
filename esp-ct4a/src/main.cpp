#include <Arduino.h>
#include <Wire.h>
#include "CONFIG.h"
#include "ct.h"
#include "MqttManager.h"
#include "connWIFI.h"

void setup() {
  Serial.begin(115200);
  delay(2000); // Allow USB CDC to enumerate
  Serial.println("\n\n=== Heat Pump Current Monitor v1.0 ===");
  if (!setupWIFI()) {
      Serial.println("WiFi setup failed. Continuing without network...");
  }
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize CT and ADS1115
  if (!setupCT()) {
      // Loop forever if init fails
      while (1) {
          delay(1000);
          Serial.print(".");
      }
  }
  Serial.println("\nSetup complete. Starting measurements...");
  Serial.println("Voltage[V]\tADC_Counts");
  Serial.println("-----------------------------------------------");
}

CT_Runtime ctState[4];

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(700);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  for (int i=0; i<4; i++) {
      // Pass the CONFIG from `sensors[i]` into the function
      ctState[i].currentValue = readCurrent(
          sensors[i].pin, 
          sensors[i].gain, 
          sensors[i].lsbVolts, 
          sensors[i].m, 
          sensors[i].b
      );
      Serial.printf("Sensor %d: Current = %.3f A\n", i, ctState[i].currentValue);
      float diff = abs(ctState[i].currentValue - ctState[i].lastReportedValue);
      if (diff > sensors[i].threshold) {
          sendMQTT(i, ctState[i].currentValue);
          ctState[i].lastReportedValue = ctState[i].currentValue;
      }
  }
}





