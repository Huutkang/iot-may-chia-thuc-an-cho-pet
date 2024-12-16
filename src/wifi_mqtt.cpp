#include "wifi_mqtt.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

// Cấu hình HiveMQ Broker
const char* mqtt_server = "9812a8f30e3c404fa505703bbdce4b3b.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "vuduychien";
const char* mqtt_pass = "123456aA";

// Chủ đề MQTT
const char* control_topic = "control";
const char* config_topic = "config";

// Trạng thái điều khiển
bool isAuto = true;  // Chế độ tự động: true, thủ công: false
bool motorStatus = false;  // Trạng thái động cơ (cho ăn): ON/OFF
int food_amount[4] = {1, 1, 1, 1};  // Lượng thức ăn cho từng bộ hẹn giờ (1 đến 10)

// Biến MQTT
bool mqtt_connected = false;
String mqttMessage = "";

// Định nghĩa MQTT và Wi-Fi
WiFiClientSecure espClient;          
PubSubClient mqttClient(espClient);

// Kết nối Wi-Fi
void setupWiFi() {
    WiFiManager wifiManager;
    if (!wifiManager.autoConnect("ESP8266_AP")) {
        delay(3000);
        ESP.restart();
    }
}

// Hàm callback xử lý message MQTT
void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (String(topic) == control_topic) {
        if (message == "ON") {
            isAuto = false;
            motorStatus = true;  // Bật động cơ (cho ăn)
        } else if (message == "AUTO") {
            isAuto = true;
            motorStatus = false;  // Chế độ tự động
        } else if (message.endsWith("off")) { 
            int timerIndex = message.substring(0, 1).toInt() - 1; // Lấy số hẹn giờ
            if (timerIndex >= 0 && timerIndex < 4) {
                food_amount[timerIndex] = 0; // Tắt bộ hẹn giờ
            }
        }
    } 
    else if (String(topic) == config_topic) {
        int timerIndex = message.substring(0, 1).toInt() - 1;  // Lấy số bộ hẹn giờ
        int amount = message.substring(2).toInt();  // Lấy số lượng thức ăn
        if (timerIndex >= 0 && timerIndex < 4 && amount >= 1 && amount <= 10) {
            food_amount[timerIndex] = amount;
        }
    }
}

// Kết nối MQTT
void connect_MQTT() {
    if (!mqttClient.connected()) {
        if (mqttClient.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
            mqtt_connected = true;
            mqttClient.subscribe(control_topic);
            mqttClient.subscribe(config_topic);
        } else {
            mqtt_connected = false;
            delay(5000);
        }
    }
}

// Setup MQTT
void setupMQTT() {
    espClient.setInsecure();
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);
    connect_MQTT();
}

void handleMQTT() {
    if (mqttClient.connected()) {
        mqttClient.loop();
    } else {
        connect_MQTT();
    }
}

void setup() {
    Serial.begin(115200);
    setupWiFi();
    setupMQTT();
}

void loop() {
    handleMQTT();

    // Thủ công: Cho ăn nếu motorStatus = true
    if (!isAuto && motorStatus) {
        Serial.println("Đang cho ăn (thủ công)...");
        delay(2000);  // Giả lập thời gian hoạt động động cơ
        motorStatus = false;  // Tắt động cơ sau khi cho ăn xong
        Serial.println("Hoàn thành cho ăn.");
    }

    // Tự động: Kiểm tra bộ hẹn giờ và thực hiện cho ăn
    if (isAuto) {
        for (int i = 0; i < 4; i++) {
            if (food_amount[i] > 0) {
                Serial.print("Bộ hẹn giờ ");
                Serial.print(i + 1);
                Serial.print(" đang cho ăn với lượng: ");
                Serial.println(food_amount[i]);
                delay(1000 * food_amount[i]);  // Giả lập cho ăn
                food_amount[i] = 0;  // Reset bộ hẹn giờ sau khi hoàn thành
            }
        }
    }
    delay(1000);  // Chờ 1 giây trước vòng lặp tiếp theo
}
