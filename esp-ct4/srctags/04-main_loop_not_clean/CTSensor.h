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
    
    // Non-blocking sampling state
    enum class State { IDLE, SAMPLING_INIT, SAMPLING_RUN, SAMPLING_DONE };
    State _state = State::IDLE;
    
    // Accumulators
    double _sumSquares = 0.0; // Use double to prevent overflow
    int _sampleCount = 0;
    const int _targetSamples = 2000;
    
    // Calibration State (Shared or per sensor?)
    // The legacy code used a global zeroOffset. 
    // We'll mimic that or calc it once.
    static float _zeroOffsetADC; 

    // Helper to process a batch of samples non-blockingly
    // Returns true if finished
    bool processBatch() {
        // Process a small batch to keep loop() responsive
        const int batchSize = 10; 
        
        for (int i = 0; i < batchSize; i++) {
            if (_sampleCount >= _targetSamples) return true;
            
            int16_t reading = _ads->readADC_SingleEnded(_config.pin);
            float acComponent = reading - _zeroOffsetADC;
            _sumSquares += acComponent * acComponent; // Accumulate in double
            _sampleCount++;
        }
        return (_sampleCount >= _targetSamples);
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

    // Start a new reading (called by Scheduler)
    void startReading() {
        if (_state == State::IDLE) {
             _state = State::SAMPLING_INIT;
        }
    }
    
    bool isIdle() {
        return _state == State::IDLE;
    }

    void loop() override {
        switch (_state) {
            case State::IDLE:
                // Do nothing, wait for Scheduler to call startReading()
                break;

            case State::SAMPLING_INIT:
                if (!_ads) { _state = State::IDLE; break; }

                // Setup ADC for this sensor
                _ads->setGain(_config.gain);
                // No need for delay(100) blocking here if we used a timer
                // but 100ms is acceptable for a one-time setup vs 15s.
                // We'll trust the scheduler gave us time.
                delay(100); 

                // Dummy flush
                _ads->readADC_SingleEnded(_config.pin);
                delay(10); 

                _sumSquares = 0.0;
                _sampleCount = 0;
                _state = State::SAMPLING_RUN;
                break;

            case State::SAMPLING_RUN:
                // Process 1 batch per loop() cycle
                if (processBatch()) {
                    _state = State::SAMPLING_DONE;
                }
                break;

            case State::SAMPLING_DONE:
                {
                    float rms_counts = sqrt(_sumSquares / _sampleCount);
                    float rms_voltage = rms_counts * _config.lsbVolts;
                    float current = (rms_voltage * 1000 - _config.b) / _config.m;
                    if (current < 0) current = 0;

                    Serial.printf("[%s] Val: %.2f A (Samples: %d)\n", _config.name, current, _sampleCount);

                    if (abs(current - _lastReportedValue) > _config.threshold) {
                        _lastReportedValue = current;
                        String payload = "{\"dev\":\"" + String(_config.name) + "\", \"val\":" + String(current, 2) + "}";
                        String topic = "sensor/" + String(_config.pin); 
                        _mqtt->publish(topic, payload);
                        Serial.printf("Reported %s: %.2f A\n", _config.name, current);
                    }
                    _state = State::IDLE;
                }
                break;
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
