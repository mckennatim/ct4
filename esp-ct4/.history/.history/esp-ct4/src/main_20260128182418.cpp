#include <Arduino.h>
#include <vector>
#include "Config.h"
#include "MqttManager.h"
#include "connWIFI.h" // Reuse existing WiFi wrapper
#include "Sensor.h"

// --- Global Infrastructure ---
WiFiClient espClient;
PubSubClient client(espClient);
MqttManager mqtt(client, DEV_ID, MQTT_USER, MQTT_PASS);

std::vector<Sensor*> sensors;

// --- Callbacks ---
#include "CTSensor.h" // Iteration 2: Include the new sensor

// --- Hardware Globals ---
Adafruit_ADS1115 ads; // The shared ADS chip

// 1. The Shim for PubSubClient
void globalMqttCallback(char* topic, byte* payload, unsigned int length) {
    mqtt.onMessage(topic, payload, length);
}

// 2. The App Logic / Router
void appMqttCallback(const String& topic, const String& payload) {
    Serial.printf("MSG: %s -> %s\n", topic.c_str(), payload.c_str());

    // Iteration 1: Catch the Time Response
    if (topic == MSG_TIME_TOPIC) {
         Serial.println(">> Time Sync Received!");
         Serial.println(payload);
         return;
    }

    // Pass to sensors
    for (auto& sensor : sensors) {
        if (sensor->handleMqttMessage(topic, payload)) return;
    }
}

// --- Factory Configuration ---
void configureSensors() {
    // 1. Initialize the Shared Hardware (ADS1115)
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
        // Skip "empty" sensors if desired, or let them run reading 0
        if (String(CT_SENSORS[i].name) != "empty") {
            sensors.push_back(new CTSensor(&mqtt, &ads, CT_SENSORS[i]));
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=== Iteration 2: CT Sensors Added ===");

    // 1. Connect WiFi
    if (!setupWIFI()) {
        Serial.println("WiFi Failed");
        // ESP.restart(); // Optional
    }

    // 2. Connect MQTT
    mqtt.begin(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(appMqttCallback);
    client.setCallback(globalMqttCallback);

    // 3. Setup Sensors (Iteration 2)
    configureSensors(); 
    
    // Only call setup() on the created objects
    for (auto& s : sensors) s->setup();
}

void loop() {
    mqtt.loop();
    for (auto& s : sensors) s->loop();
}
