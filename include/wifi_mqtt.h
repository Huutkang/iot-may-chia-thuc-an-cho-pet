#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

#include <Arduino.h>


extern bool isAuto;
extern bool status;
extern bool mqtt_connected;
extern int foodAmount;
extern String mqttMessage;

void setupWiFi();
void setupMQTT();
void connect_MQTT();
bool publishData(const char* topic, const char* payload);
void handleMQTT();


#endif
