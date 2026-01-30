#include <Arduino.h>
#include <vector>
#include "Config.h"
#include "MqttManager.h"
#include "connWIFI.h" // Reuse existing WiFi wrapper
#include "Sensor.h"
#include "ProjectFactory.h" // <-- Defines configureSensors() and project globals

// --- Global Infrastructure ---
WiFiClient espClient;
PubSubClient client(espClient);
MqttManager mqtt(client, DEV_ID, MQTT_USER, MQTT_PASS);

std::vector<Sensor*> sensors;

// --- Callbacks ---

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

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=== Iteration 3: Generic Main ===");

    // 1. Connect WiFi
    if (!setupWIFI()) {
        Serial.println("WiFi Failed");
        // ESP.restart(); // Optional
    }

    // 2. Connect MQTT
    mqtt.begin(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(appMqttCallback);
    client.setCallback(globalMqttCallback);

    // 3. Setup Sensors (Project Factory)
    configureSensors(sensors, &mqtt); 
    
    // Only call setup() on the created objects
    for (auto& s : sensors) s->setup();

    Serial.println("Setup Complete, entering loop...");
}

// Round-Robin Scheduler State
unsigned long lastSensorRunTime = 0;
size_t currentSensorIndex = 0;
// Run one sensor every 3 seconds (giving 3s of pure MQTT time between blocks)
const unsigned long SENSOR_INTERVAL = 3000; 

void loop() {
    // 1. Always run MQTT (KeepAlive & Messages)
    mqtt.loop();

    // 2. Scheduled Sensor Execution (One at a time)
    unsigned long now = millis();
    if (now - lastSensorRunTime > SENSOR_INTERVAL) {
        lastSensorRunTime = now;

        if (!sensors.empty()) {
            Serial.printf("[Scheduler] Running Sensor %d\n", currentSensorIndex);
            
            // This call will block for ~1.2s (due to CT sampling)
            // But since we only run ONE, the total block is 1.2s, not 4.8s.
            sensors[currentSensorIndex]->loop();

            // Move to next sensor for next slot
            currentSensorIndex++;
            if (currentSensorIndex >= sensors.size()) {
                currentSensorIndex = 0;
            }
        }
    }
}
