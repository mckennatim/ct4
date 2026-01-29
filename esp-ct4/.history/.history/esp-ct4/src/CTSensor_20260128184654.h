#pragma once
#include "Sensor.h"
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include "Config.h"
#include "MqttManager.h"

// Provide access to the shared instance if needed globally, 
// or pass it in. We'll pass it in.

class CTSensor : public Sensor {
private:
    Adafruit_ADS1115* _ads;
    MqttManager* _mqtt;
    CT_Config _config;
    
    // Runtime state
    float _lastReportedValue = 0.0;
    unsigned long _lastReadTime = 0;
    const unsigned long _readInterval = 2000; // Read every 2s (example)

    // Calibration State (Shared or per sensor?)
    // The legacy code used a global zeroOffset. 
    // We'll mimic that or calc it once.
    static float _zeroOffsetADC; 
    
    float readCurrent() {
        if (!_ads) return 0.0;
        
        _ads->setGain(_config.gain);
        // Note: Legacy code had heavy delays here. 
        // We are keeping them for now to ensure accuracy, 
        // but this stops the loop. 
        
        // Dummy read to flush
        _ads->readADC_SingleEnded(_config.pin);
        delay(10); 

        long sumSquares = 0;
        const int samples = 500; // Reduced from 2000 for faster loop response in iteration 2
        
        for (int i = 0; i < samples; i++) {
            int16_t reading = _ads->readADC_SingleEnded(_config.pin);
            float acComponent = reading - _zeroOffsetADC;
            sumSquares += acComponent * acComponent;
            // logic to unblock wifi could go here if samples are high
        }

        float rms_counts = sqrt(sumSquares / samples);
        float rms_voltage = rms_counts * _config.lsbVolts;
        float rms_current = (rms_voltage * 1000 - _config.b) / _config.m;
        
        if (rms_current < 0) rms_current = 0;
        return rms_current;
    }

public:
    CTSensor(MqttManager* mqtt, Adafruit_ADS1115* ads, CT_Config config)
        : _mqtt(mqtt), _ads(ads), _config(config) {}

    void setup() override {
        // Individual setup if needed
    }

    // Static helper to calibrate the shared chip
    static void calibrateZero(Adafruit_ADS1115* ads) {
        Serial.println("Calibrating Probe Zero Offset...");
        ads->setGain(GAIN_ONE);
        long sum = 0;
        for(int i=0; i<500; i++) {
            sum += ads->readADC_SingleEnded(0); // Assuming Ch0 is reference or idle?
            delay(2);
        }
        _zeroOffsetADC = sum / 500.0;
        Serial.printf("Zero Offset: %.2f\n", _zeroOffsetADC);
    }

    void loop() override {
        unsigned long now = millis();
        // Stagger reads based on pin index to prevent easier bursts? 
        // For now just simple interval.
        if (now - _lastReadTime > _readInterval) {
            _lastReadTime = now;
            
            float current = readCurrent();
            
            // Debug Log
            // Serial.printf("[%s] Current: %.2f A\n", _config.name, current);

            // Report if changed significantly
            if (abs(current - _lastReportedValue) > _config.threshold) {
                _lastReportedValue = current;
                
                // Construct JSON payload
                // {"dev":"Heat_Pump", "val": 12.5}
                String payload = "{\"dev\":\"" + String(_config.name) + "\", \"val\":" + String(current, 2) + "}";
                String topic = "sensor/" + String(_config.pin); 
                
                _mqtt->publish(topic, payload);
                Serial.printf("Reported %s: %.2f A\n", _config.name, current);
            }
        }
    }
};

// Define the static member
float CTSensor::_zeroOffsetADC = 0.0;

class CTFactory {
public:
    static void load(std::vector<Sensor*>& sensors, MqttManager* mqtt) {
        #ifdef USE_CT_SENSORS
        // Access the hardware singleton
        static Adafruit_ADS1115 ads;
        static bool initialized = false;

        if (!initialized) {
            Wire.begin(I2C_SDA, I2C_SCL);
            if (!ads.begin(0x48, &Wire)) {
                Serial.println("Failed to init ADS1115!");
                return;
            }
            Serial.println("ADS1115 Initialized.");
            CTSensor::calibrateZero(&ads);
            initialized = true;
        }

        // Iterate through the Config Array
        int numSensors = sizeof(ct_sensors) / sizeof(ct_sensors[0]);
        for (int i = 0; i < numSensors; i++) {
             if (String(ct_sensors[i].name) != "empty") {
                 sensors.push_back(new CTSensor(mqtt, &ads, ct_sensors[i]));
             }
        }
        #endif
    }
};
