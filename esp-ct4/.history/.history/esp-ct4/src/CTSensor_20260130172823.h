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
        
        // Debug: Log start of read
        // Serial.printf("Reading CT: %s (pin %d)... ", _config.name, _config.pin);
        
        _ads->setGain(_config.gain);
        // _ads->setDataRate(RATE_ADS1115_860SPS); // Removed: High speed caused underreporting due to impedance
        
        // Critical Settling Time for highly inductive loads/long wires
        delay(100); 

        // Dummy read to flush
        _ads->readADC_SingleEnded(_config.pin);
        delay(10); 

        long sumSquares = 0;
        // BACK TO BASICS: 128SPS (Default).
        // 150 samples @ 128SPS = ~1.2 seconds.
        // This is safe for MQTT keepalive (usually 15s) but gives 
        // ~70 full 60Hz cycles for accurate RMS.
        const int samples = 150; 
        
        for (int i = 0; i < samples; i++) {
            int16_t reading = _ads->readADC_SingleEnded(_config.pin);
            float acComponent = reading - _zeroOffsetADC;
            sumSquares += acComponent * acComponent;
        }

        float rms_counts = sqrt(sumSquares / samples);
        float rms_voltage = rms_counts * _config.lsbVolts;
        float rms_current = (rms_voltage * 1000 - _config.b) / _config.m;
        
        if (rms_current < 0) rms_current = 0;
        
        // Serial.printf("Done. Val: %.2f\n", rms_current);
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
            Serial.printf("[%s] checking... Val: %.2f A\n", _config.name, current);

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
        Serial.println(">>> CTFactory::load (Initializing Hardware)");
        
        // 1. Initialize Hardware (Single Instance)
        // Memory Leak Warning: This pointer is never deleted, but it lives forever so OK.
        Adafruit_ADS1115* ads = new Adafruit_ADS1115(); 
        
        Serial.printf(">>> CTFactory: Initializing I2C (SDA: %d, SCL: %d)\n", I2C_SDA, I2C_SCL);
        Wire.begin(I2C_SDA, I2C_SCL); 
        Wire.setClock(400000); // Fast Mode (400kHz) for simpler communication
        
        if (!ads->begin()) {
            Serial.println("!!! Failed to initialize ADS1115 via I2C !!!");
            // If I2C fails, we can't do anything with CT sensors.
            return;
        }

        // 2. Calibrate once
        CTSensor::calibrateZero(ads);

        // 3. Create Sensors based on ct_sensors array
        for (const auto& cfg : ct_sensors) {
            if (String(cfg.name) == "empty") {
                Serial.printf("Skipping empty slot %d\n", cfg.pin);
                continue;
            }

            Serial.printf("Creating CT Sensor: %s (Pin %d)\n", cfg.name, cfg.pin);
            CTSensor* s = new CTSensor(mqtt, ads, cfg);
            sensors.push_back(s);
        }
        Serial.println(">>> CTFactory::load Complete");
    }
};
