#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <functional>

// 1. Define a callback type for your app logic (Topic, Payload)
using MqttAppCallback = std::function<void(const String& topic, const String& payload)>;

class MqttManager {
private:
    PubSubClient& _client;
    String _deviceId;
    String _server;
    uint16_t _port;
    String _user;
    String _password;
    
    // Config
    String _baseTopic; // e.g. "CYURD127"
    
    // State
    unsigned long _lastReconnectAttempt = 0;
    const unsigned long _reconnectInterval = 5000;
    MqttAppCallback _appCallback = nullptr;

    // Internal helper to subscribe to standard topics
    void _subscribeToTopics() {
        // Subscribe to base actions
        String subPattern = _deviceId + "/#"; // Wildcard subscribe to everything for this device?
        _client.subscribe(subPattern.c_str());
        // Or strictly:
        // _client.subscribe((_deviceId + "/time").c_str());
    }

public:
    MqttManager(PubSubClient& client, const char* devId, const char* user, const char* pwd)
        : _client(client), _deviceId(devId), _user(user), _password(pwd) {}

    // Setup the server details
    void begin(const char* server, uint16_t port) {
        _server = server;
        _port = port;
        _client.setServer(server, port);
    }

    // Set the function that handles valid incoming messages
    void setCallback(MqttAppCallback cb) {
        _appCallback = cb;
    }

    // This must be called from the global PubSubClient callback
    void onMessage(char* topic, uint8_t* payload, unsigned int length) {
        // 1. Convert to safe strings immediately
        String safeTopic = String(topic);
        String safePayload = "";
        for (unsigned int i = 0; i < length; i++) {
            safePayload += (char)payload[i];
        }

        // 2. Validate or Clean? 
        // Example: Only pass on if it starts with our DeviceID
        if (safeTopic.startsWith(_deviceId)) {
            // Remove the DeviceID prefix if you want just the command (e.g., "CYURD127/cmd" -> "cmd")
            String relativeTopic = safeTopic.substring(_deviceId.length() + 1); // +1 for '/'
            
            // 3. Delegate to the app
            if (_appCallback) {
                _appCallback(relativeTopic, safePayload);
            }
        }
    }

    // Main loop - Call this in the Arduino loop()
    void loop() {
        if (!_client.connected()) {
            unsigned long now = millis();
            // Non-blocking reconnect timer
            if (now - _lastReconnectAttempt > _reconnectInterval) {
                _lastReconnectAttempt = now;
                Serial.print("Attempting MQTT connection...");
                
                if (_client.connect(_deviceId.c_str(), _user.c_str(), _password.c_str())) {
                    Serial.println("connected");
                    _subscribeToTopics();
                    
                    // Announce arrival
                    publish("status", "online");
                } else {
                    Serial.print("failed, rc=");
                    Serial.print(_client.state());
                    Serial.println(" (retrying in 5s)");
                }
            }
        } else {
            _client.loop(); // Allow PubSubClient to process incoming
        }
    }

    // Helper to publish simply: deviceID/subtopic
    void publish(String subtopic, String message) {
        if (_client.connected()) {
            String fullTopic = _deviceId + "/" + subtopic;
            _client.publish(fullTopic.c_str(), message.c_str());
        }
    }
};

void sendMQTT(int sensorIndex, float currentValue);

