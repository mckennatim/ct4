#include <Arduino.h>
#include "MqttManager.h"

MqttManager::MqttManager(PubSubClient& client, const char* devId, const char* user, const char* pwd)
    : _client(client), _deviceId(devId), _user(user), _password(pwd), _lastReconnectAttempt(0), _appCallback(nullptr) {}

void MqttManager::begin(const char* server, uint16_t port) {
    _server = server;
    _port = port;
    _client.setServer(server, port);
}

void MqttManager::setCallback(MqttAppCallback cb) {
    _appCallback = cb;
}

void MqttManager::_subscribeToTopics() {
    String subPattern = _deviceId + "/#"; 
    _client.subscribe(subPattern.c_str());
}

void MqttManager::loop() {
    if (!_client.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > _reconnectInterval) {
            _lastReconnectAttempt = now;
            if (_client.connect(_deviceId.c_str(), _user.c_str(), _password.c_str())) {
                _subscribeToTopics();
                publish("status", "online");
            }
        }
    } else {
        _client.loop();
    }
}

void MqttManager::onMessage(char* topic, uint8_t* payload, unsigned int length) {
    String safeTopic = String(topic);
    String safePayload = "";
    for (unsigned int i = 0; i < length; i++) {
        safePayload += (char)payload[i];
    }

    if (safeTopic.startsWith(_deviceId)) {
        String relativeTopic = safeTopic.substring(_deviceId.length() + 1); 
        if (_appCallback) {
            _appCallback(relativeTopic, safePayload);
        }
    }
}

void MqttManager::publish(String subtopic, String message) {
    if (_client.connected()) {
        String fullTopic = _deviceId + "/" + subtopic;
        _client.publish(fullTopic.c_str(), message.c_str());
    }
}


void sendMQTT(int sensorIndex, float currentValue) {
    Serial.printf("Sending MQTT for sensor %d: %.3f A\n", sensorIndex, currentValue);
} 