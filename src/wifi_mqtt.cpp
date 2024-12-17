#include "wifi_mqtt.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

// Cấu hình HiveMQ Broker
const char* mqtt_server = "2e39b02babb14c8ebf5dc699965ed7f7.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "nguyenvanson";
const char* mqtt_pass = "Nguyenvanson123@";

// Chủ đề MQTT
const char* control_topic = "control";
const char* config_topic = "config";

// Biến MQTT
bool mqtt_connected = false;
String mqttMessage = "";

int count_connect_wifi = 0;

// Trạng thái điều khiển
bool isAuto = true;  // Chế độ tự động: true, thủ công: false
bool status = false;  // Trạng thái động cơ (cho ăn): ON/OFF
int foodAmount = 5;  // Lượng thức ăn cho ăn trong mỗi lần


// Định nghĩa MQTT và Wi-Fi
WiFiClientSecure espClient;          // Đối tượng WiFiClient
PubSubClient mqttClient(espClient);  // Đối tượng MQTT client

void setupWiFi() {
    WiFiManager wifiManager;

    // Tự động kết nối hoặc tạo Access Point
    if (!wifiManager.autoConnect("ESP8266_AP")) {
        // Serial.println("Không kết nối được Wi-Fi");
        delay(3000);
        ESP.restart();  // Khởi động lại thiết bị
    }
    // Serial.println("Đã kết nối Wi-Fi!");
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (String(topic) == control_topic) {
        if (message.startsWith("FEED")) {
            status = true;  // cho thú cưng ăn
        } else if (message.startsWith("ON")) {
            isAuto = true; // Bật tự động cho thú cưng ăn
        } else if (message == "OFF") {
            isAuto = false; // Tắt tự động cho thú cưng ăn
        }
    } else if (String(topic) == config_topic) {
        if (message.startsWith("CA")) {
            int newFoodAmount = message.substring(3).toInt();
            if (newFoodAmount > 0 && newFoodAmount < 11) {
                foodAmount = newFoodAmount;
            }
        }else{
            mqttMessage = message; // Lưu lại toàn bộ chuỗi nhận được, phục vụ cho bộ hẹn giờ
        }
    }
}

void connect_MQTT() {
    if (!mqttClient.connected()) {
        Serial.print("Đang kết nối MQTT...");
        if (mqttClient.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
            mqtt_connected = true;
            // Serial.println("Đã kết nối MQTT!");
            mqttClient.subscribe(control_topic);
            mqttClient.subscribe(config_topic);
        } else {
            // Serial.print("Lỗi MQTT: ");
            // Serial.println(mqttClient.state());
            if (WiFi.status() != WL_CONNECTED || count_connect_wifi > 5) {
                count_connect_wifi = 0;
                // Serial.println("Lỗi wifi");
                WiFi.reconnect();
            } else {
                count_connect_wifi++;
            }
            for (int i = 0; i < 4; i++) {
                isAuto = true;
            }
        }
    }
}

void setupMQTT() {
    espClient.setInsecure();
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);
    connect_MQTT();
}

bool publishData(const char* topic, const char* payload) {
    mqttClient.loop();
    if (mqttClient.connected()) {
        mqttClient.publish(topic, payload);
        return true;
    } else {
        mqtt_connected = false;
        return false;
    }
}

void handleMQTT() {
    if (mqttClient.connected()) {
        mqttClient.loop();
    } else {
        mqtt_connected = false;
    }
}
