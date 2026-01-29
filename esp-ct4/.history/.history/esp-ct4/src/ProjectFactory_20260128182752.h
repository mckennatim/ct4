#pragma once
#include <Arduino.h>
#include <vector>
#include "Config.h"
#include "MqttManager.h"
#include "Sensor.h"

// --- Project Specific Sensors ---
#include "CTSensor.h"

// --- Shared Hardware Globals ---
// This object lives here, specific to THIS project configuration.
// If your next project doesn't use ADS1115, you remove this line 
// and the CTSensor includes.
Adafruit_ADS1115 ads; 

inline void configureSensors(std::vector<Sensor*>& sensors, MqttManager* mqtt) {
    // 1. Initialize Shared Hardware
    // Uses the pins defined in Config.h
    // If you don't have I2C, you simply delete these lines in the next project.
    Wire.begin(I2C_SDA, I2C_SCL); 
    
    if (!ads.begin(0x48, &Wire)) {
        Serial.println("Failed to init ADS1115!");
    } else {
        Serial.println("ADS1115 Initialized.");
        CTSensor::calibrateZero(&ads); 
    }

    // 2. Create Sensor Objects from Config Data
    int numSensors = sizeof(CT_SENSORS) / sizeof(CT_SENSORS[0]);
    for (int i = 0; i < numSensors; i++) {
        if (String(CT_SENSORS[i].name) != "empty") {
            sensors.push_back(new CTSensor(mqtt, &ads, CT_SENSORS[i]));
        }
    }
}
