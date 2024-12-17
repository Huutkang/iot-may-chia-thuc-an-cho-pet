#include <Arduino.h>
extern "C" {
  #include "osapi.h"
  #include "os_type.h"
}
#include "wifi_mqtt.h"
#include "sensor.h"
#include "motor.h"
#include "time_sync.h"
#include "speaker.h"


// Chỉ thị tiền sử lí, bắt đầu bằng #. đây không phải biến
// vd: #define abc xyz
// trình biên dịch sẽ đổi tên những chỗ nào có abc bằng xyz trong mã nguồn trước khi biên dịch

os_timer_t motorTimer; // Khai báo Software Timer

// Khai báo các chân điều khiển động cơ
#define IN1 D6 
#define IN2 D8
#define IN3 D2
#define IN4 D1

// Cái còi để gọi thú cưng
#define SPEAKER D0

// Cảm biến xem còn hay hết thức ăn
#define FOODSENSOR D5
// Cảm biến chuyển động
#define MOTIONSENSOR D7


// các biến lưu giữ thời gian, phục vụ cho hàm hẹn giờ
unsigned long current_time;
unsigned long time1=0;
unsigned long time2=0;
unsigned long time3=0;
unsigned long time4=0;
unsigned long time5=0;
unsigned long time6=0;
unsigned long time7=0;

bool hasSetup = false; // đã gọi thú cưng


// Hàm callback cho Software Timer
void motorTimerCallback(void *pArg) {
    if (motorOn) {
        rotateMotor(); // Quay động cơ
    }
}

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
    if (isAuto) {
        if (feeding_timer) {
            status = true;
            feeding_timer = false; // Reset lại bộ hẹn giờ
            
        }
    }
    if (status && !hasSetup){
        int revolutions = foodAmount/2; // mỗi mức cho ăn quay nữa vòng, 1/2 là hằng số cho ăn, có thể đổi
        setRotate(revolutions);
        setCalls(5);
        speakerOn = true;
        hasSetup = true;
    }
    if (status && sensor[0]){
        setCalls(1);
        speakerOn = true;
        status = false;
        motorOn = true;
        hasSetup = false;
    }
}


void setup() {
    setupSpeaker(SPEAKER);
    setupMotor(IN1, IN2, IN3, IN4);
    setupSensors(MOTIONSENSOR, FOODSENSOR);
    Serial.begin(115200);
    setupWiFi();                  // Cấu hình WiFi
    setupMQTT();                 // Cấu hình MQTT 
    setupTimeSync();

    // Cấu hình Software Timer cho động cơ
    os_timer_setfn(&motorTimer, motorTimerCallback, NULL);
    os_timer_arm(&motorTimer, 1, true); // Thời gian là 1ms, lặp lại liên tục
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
        if (sensor[0]){
            publishData("test", "MOTIONSENSOR OK");
        }
        if (!sensor[1]){
            publishData("test", "FOODSENSOR OK");
        }         
    }
    if (Timer(&time3,400)){ // thay đổi trạng thái
        updateStatus(); // thay đổi trạng thái máy cho ăn
        
    }
    if (Timer(&time4,991)){ // hẹn giờ
        ProcessTimerString(mqttMessage); // hẹn giờ cho ăn
        checkAndActivateTimers();  // kích hoạt các máy cho ăn đã hẹn giờ
    }
    if (Timer(&time5, waitSpeaker)){ // loa
        if (speakerOn){
            callPet(); // gọi Pet
        }
    }
    
    
    // Đồng bộ thời gian mỗi 15 phút
    updateTimeSync();
}
