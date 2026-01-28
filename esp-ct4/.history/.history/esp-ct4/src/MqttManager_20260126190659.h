#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <functional>

using MqttAppCallback = std::function<void(const String& topic, const String& payload)>;

class MqttManager {
private:
    PubSubClient& _client;
    String _deviceId;
    String _server;
    uint16_t _port;
    String _user;
    String _password;
    String _baseTopic;
    
    unsigned long _lastReconnectAttempt;
    const unsigned long _reconnectInterval = 5000;
    MqttAppCallback _appCallback;

    void _subscribeToTopics();

public:
    MqttManager(PubSubClient& client, const char* devId, const char* user, const char* pwd);
    void begin(const char* server, uint16_t port);
    void setCallback(MqttAppCallback cb);
    void onMessage(char* topic, uint8_t* payload, unsigned int length);
    void loop();
    void publish(String subtopic, String message);
};