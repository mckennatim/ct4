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
         // You can parse JSON here if needed
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
    Serial.println("\n\n=== Iteration 1: Clean Architecture ===");

    // 1. Connect WiFi
    if (!setupWIFI()) {
        Serial.println("WiFi Failed");
        // ESP.restart(); // Optional
    }

    // 2. Connect MQTT
    mqtt.begin(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(appMqttCallback);
    client.setCallback(globalMqttCallback);

    // 3. Setup Sensors (None for Iteration 1)
    // configureSensors(sensors, &mqtt); 
    for (auto& s : sensors) s->setup();
}

void loop() {
    mqtt.loop();
    for (auto& s : sensors) s->loop();
}
