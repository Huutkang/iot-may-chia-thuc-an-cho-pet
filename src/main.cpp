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

int t[4] = {max_time[0], max_time[1], max_time[2], max_time[3]};
int ActivationTime = 60;
bool count_status[4] = {false, false, false, false};


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
    for (int i = 0; i < 4; i++) {
        if (!isAuto[i]) {
            // Nếu không ở chế độ tự động, bỏ qua máy bơm này
            continue;
        }
        if (t[i]<0){
            status[i] = false;
        }

        if (sensor[i] <= min_moisture[i]) {
            if (!count_status[i]){
                count_status[i] = true;
                status[i] = true;
            }
        } else if (sensor[i] >= max_moisture[i]) {
            status[i] = false; // Không tưới
        } else {
            // Độ ẩm nằm trong khoảng 60-90
            if (watering_timer[i]) {
                if (!count_status[i]){
                    count_status[i] = true;
                    status[i] = true;
                }
                watering_timer[i] = false; // Reset lại bộ hẹn giờ
            }
        }
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
    if (Timer(&time2,5000)){ // đọc, gửi, in giá trị cảm biến
        readSensors();                
        for (int i = 0; i < 4; i++) {
            String message = String(i + 1) + " " + String(sensor[i]);
            publishData("RH", message.c_str());
        }
        String pump_status = String(status[0]) + String(status[1]) + String(status[2]) + String(status[3]);
        publishData("PS", pump_status.c_str());
    }
    if (Timer(&time3,500)){ // thực thi bật tắt máy bơm
        manageMotor(MT, status);
    }
    if (Timer(&time4,990)){ // thay đổi trạng thái
        updateStatus(); // thay đổi trạng thái máy bơm
        for (int i=0; i<4; i++){ // kiểm soát thời gian tưới tối đa và thời gian tối thiểu từ khi tắt đến khi bật (chế độ auto)
            if (count_status[i]){
                t[i]--; // bắt buộc để timer ở đây là 1 giây để hoạt động bình thường
            }
            if (t[i]<-ActivationTime){
                count_status[i]=false;
                t[i]=max_time[i];
            }
        }
    }
    if (Timer(&time5,991)){ // hẹn giờ
        ProcessTimerString(mqttMessage); // hẹn giờ bơm
        checkAndActivateTimers();  // kích hoạt các máy bơm đã hẹn giờ
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
