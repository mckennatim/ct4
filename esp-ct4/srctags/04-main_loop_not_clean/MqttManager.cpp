#include "MqttManager.h"
#include "Config.h" 

MqttManager::MqttManager(PubSubClient& client, const char* devId, const char* user, const char* pwd)
    : _client(client), _deviceId(devId), _user(user), _password(pwd), _lastReconnectAttempt(0), _appCallback(nullptr) {}

void MqttManager::begin(const char* server, uint16_t port) {
    _client.setServer(server, port);
}

void MqttManager::setCallback(MqttAppCallback cb) {
    _appCallback = cb;
}

void MqttManager::_subscribeToTopics() {
    String subPattern = _deviceId + "/#"; 
    _client.subscribe(subPattern.c_str());
}

void MqttManager::_onConnect() {
    _subscribeToTopics();
    Serial.println("MQTT Connected");
    
    // Iteration 1: Send the Time Sync Message
    #ifdef MSG_TIME_TOPIC
    Serial.println("Sending Time Request...");
    publish(MSG_TIME_TOPIC, MSG_TIME_PAYLOAD);
    #endif
}

void MqttManager::loop() {
    if (!_client.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > _reconnectInterval) {
            _lastReconnectAttempt = now;
            // Attempt to connect
            Serial.print("MqttManager: Attempting connection to ");
            Serial.print("...");
            
            if (_client.connect(_deviceId.c_str(), _user.c_str(), _password.c_str())) {
                _onConnect();
            } else {
                Serial.print("failed, rc=");
                Serial.println(_client.state());
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
    } else {
        // Fallback for strict matches or broadcasts if needed
        // For now, only handle device topics
    }
}

void MqttManager::publish(String subtopic, String message) {
    if (_client.connected()) {
        String fullTopic = _deviceId + "/" + subtopic;
        _client.publish(fullTopic.c_str(), message.c_str(), true); // Retain=true per legacy example
    }
}
