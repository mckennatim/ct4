#pragma once
#include "Sensor.h"
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include "Config.h"
#include "MqttManager.h"

class CTSensor : public Sensor {
private:
    Adafruit_ADS1115* _ads;
    MqttManager* _mqtt;
    CT_Config _config;
    
    // Runtime state
    float _lastReportedValue = -1.0;
    
    // Static Token State
    static int _nextTurnIndex;
    static int _totalSensors;

    const int _targetSamples = 1600; 
    static float _zeroOffsetADC; 

public:
    CTSensor(MqttManager* mqtt, Adafruit_ADS1115* ads, CT_Config config)
        : _mqtt(mqtt), _ads(ads), _config(config) {
            _totalSensors++;
        }

    // ... setup() ... 

    void loop() override {
        // --- The Gatekeeper ---
        // Use '_pin' as the ID since it maps 0-3 on the ADS1115
        if (_config.pin != _nextTurnIndex) {
            return;
        }

        // --- My Turn (Blocking) ---
        _ads->setGain(_config.gain);
        _ads->setDataRate(RATE_ADS1115_860SPS);
        _ads->readADC_SingleEnded(_config.pin); // Flush

        double sumSquares = 0.0;
        for (int i = 0; i < _targetSamples; i++) {
            int16_t reading = _ads->readADC_SingleEnded(_config.pin);
            float acComponent = reading - _zeroOffsetADC;
            sumSquares += acComponent * acComponent;
        }

        float rms = sqrt(sumSquares / _targetSamples) * _config.lsbVolts;
        float current = (rms * 1000 - _config.b) / _config.m;
        if (current < 0) current = 0;

        Serial.printf("Sensor %d: Current = %.3f A\n", _config.pin, current);
        
        float diff = abs(current - _lastReportedValue);
        if (diff > _config.threshold) {
             Serial.printf("Sending MQTT for sensor %d: %.3f A\n", _config.pin, current);
             _mqtt->publish((String("home/sensors/ct") + _config.pin + "/current").c_str(), String(current));
             _lastReportedValue = current;
        }

        // --- Pass the Token ---
        _nextTurnIndex++;
        // NOTE: We rely on the factory to populate sensors in order [0,1,2...]
        // If there are holes (skipped empty), this token ring might break if not careful.
        // A safer way is modulo the known total.
        if (_nextTurnIndex >= _totalSensors) {
            _nextTurnIndex = 0;
        }
    }

// Define statics
int CTSensor::_nextTurnIndex = 0;
int CTSensor::_totalSensors = 0;
float CTSensor::_zeroOffsetADC = 0.0;
// bool CTSensor::_i2cBusy = false; // Not strictly needed with token passing, but good hygiene