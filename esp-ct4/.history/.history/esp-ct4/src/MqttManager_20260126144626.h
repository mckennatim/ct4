#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <functional>

void sendMQTT(int sensorIndex, float currentValue);

