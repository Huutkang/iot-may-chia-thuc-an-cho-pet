#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

#include <Arduino.h>


extern bool isAuto[4];
extern bool status[4];
extern bool mqtt_connected;
extern int min_moisture[4];
extern int max_moisture[4];
extern int max_time[4];
extern String mqttMessage;

void setupWiFi();
void setupMQTT();
void connect_MQTT();
bool publishData(const char* topic, const char* payload);
void ProcessTimerString(const String& input);
void handleMQTT();


#endif
