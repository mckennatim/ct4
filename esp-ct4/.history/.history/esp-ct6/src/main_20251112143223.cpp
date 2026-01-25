#include <Arduino.h>
#include "EmonLib-ESP32.h"  // Use quotes, not angle brackets

EnergyMonitor ct1;  // Now you can create instances

// put function declarations here:
int myFunction(int, int);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  int result = myFunction(2, 3);
  ct1.current(32, 111.1);  // GPIO 32
}

void loop() {
  Serial.println("ESP32 is running!");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  double Irms = ct1.calcIrms(1480);
  Serial.println(Irms);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}