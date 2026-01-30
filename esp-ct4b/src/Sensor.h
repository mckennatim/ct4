#pragma once
#include <Arduino.h>

class Sensor {
public:
    virtual ~Sensor() {}
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual bool handleMqttMessage(const String& subtopic, const String& payload) {
        return false; 
    }
};
