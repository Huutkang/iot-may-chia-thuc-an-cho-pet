#include <Arduino.h>
#include "wifi_mqtt.h"
#include "sensor.h"
#include "motor.h"
#include "time_sync.h"
#include "speaker.h"


// Chỉ thị tiền sử lí, bắt đầu bằng #. đây không phải biến
// vd: #define abc xyz
// trình biên dịch sẽ đổi tên những chỗ nào có abc bằng xyz trong mã nguồn trước khi biên dịch

// Khai báo các chân điều khiển động cơ
#define IN1 D6 
#define IN2 D8
#define IN3 D2
#define IN4 D1

// Cái còi để gọi thú cưng
#define SPEAKER D0

// Cảm biến chuyển động
#define MOTIONSENSOR D5
// Cảm biến xem còn hay hết thức ăn
#define FOODSENSOR D7

// các biến lưu giữ thời gian, phục vụ cho hàm hẹn giờ
unsigned long current_time;
unsigned long time1=0;
unsigned long time2=0;
unsigned long time3=0;
unsigned long time4=0;
unsigned long time5=0;
unsigned long time6=0;
unsigned long time7=0;



// với kiểu dữ liệu unsigned long: 10 - 4294967295 = 11 nên không lo tràn số ở hàm millis nhé
// Ưu điểm: không dùng delay
int Timer(unsigned long *time, int wait){
    current_time = millis();
    if (current_time-*time>wait){
        *time = current_time;
        return 1;
    }
    else{
        return 0;
    }
}

void updateStatus() {
    if (!isAuto) {
        return;
    }
    if (feeding_timer) {
        motorStatus = true;
        feeding_timer = false; // Reset lại bộ hẹn giờ
    }
}


void setup() {
    setupSpeaker(int SPEAKER);
    setupMotor(IN1, IN2, IN3, IN4);
    setupSensors(MOTIONSENSOR, FOODSENSOR);
    Serial.begin(115200);
    setupWiFi();                  // Cấu hình WiFi
    setupMQTT();                 // Cấu hình MQTT 
    setupTimeSync();
}

void loop() {
    handleMQTT();                 // Xử lý kết nối MQTT
    if (Timer(&time1, 3000)){ // kết nối lại mqtt, wifi
        if(!mqtt_connected){
            connect_MQTT();
        }
    }
    if (Timer(&time2,500)){
        readSensors();                
    }
    if (Timer(&time3,500)){
        // tính toán góc

        // bật động cơ
        setRotate(float revolutions);
    }
    if (Timer(&time4,990)){ // thay đổi trạng thái
        updateStatus(); // thay đổi trạng thái máy cho ăn
        
    }
    if (Timer(&time5,991)){ // hẹn giờ
        ProcessTimerString(mqttMessage); // hẹn giờ cho ăn
        checkAndActivateTimers();  // kích hoạt các máy cho ăn đã hẹn giờ
    }
    if (Timer(&time6, waitSpeaker)){ // loa
        if (speakerOn){
            callPet(); // gọi Pet
        }
    }
    if (Timer(&time7, 1)){ // chờ 1 mili giây để nhảy sang step tiếp theo của động cơ. cần 4096 bước để quay hết một vòng
        if (motorOn){
            rotateMotor(); // quay động cơ
        }
    }
    
    // Đồng bộ thời gian mỗi 15 phút
    updateTimeSync();
}
