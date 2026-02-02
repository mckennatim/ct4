#pragma once
#include <Arduino.h>
#include <vector>
#include "Config.h"
#include "MqttManager.h"
#include "Sensor.h"

// --- Feature Modules ---
// Only include the modules if you enabled them in Config
#ifdef USE_CT_SENSORS
  #include "CTSensor.h"
#endif

inline void configureSensors(std::vector<Sensor*>& sensors, MqttManager* mqtt) {
    
    // Load CT Module if enabled
    #ifdef USE_CT_SENSORS
        CTFactory::load(sensors, mqtt);
    #endif
}
