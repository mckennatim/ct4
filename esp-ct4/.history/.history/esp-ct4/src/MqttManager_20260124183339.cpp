#include "mqqt.h"
#include <Arduino.h>

void sendMQTT(int sensorIndex, float currentValue) {
    Serial.printf("Sending MQTT for sensor %d: %.3f A\n", sensorIndex, currentValue);
} 