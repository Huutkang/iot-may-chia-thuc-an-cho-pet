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

bool hasSetup = false; // đã gọi thú cưng
bool motionNoti = false; // thông báo cử động
int totalFood = 0;
int hungry = 0; // dành cho thông báo thú cưng đói

// Hàm callback cho Software Timer (Timer phần mềm của hệ thống, esp8266 có duy nhất 1 timer phần cứng và được dùng cho wifi rồi)
// dùng riêng cái này cho động cơ, dùng hàm Timer bên dưới thì nó lag. còn nếu tất cả dùng timer này thì cũng lag như nhau
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
        int revolutions = foodAmount*2/3; // mỗi mức cho ăn quay nữa vòng, 2/3 là hằng số cho ăn, có thể đổi
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
        totalFood = totalFood + foodAmount;
        publishData("chart", String(foodAmount).c_str());
        hungry = 1440;
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
            motionNoti = true;   
        }  
    }
    if (Timer(&time3,400)){ // thay đổi trạng thái
        updateStatus(); // thay đổi trạng thái máy cho ăn
        
    }
    if (Timer(&time4,991)){ // hẹn giờ
        ProcessTimerString(mqttMessage); // hẹn giờ cho ăn
        checkAndActivateTimers();  // kích hoạt các máy cho ăn đã hẹn giờ
        if (getSecondsSinceMidnight()<2){
            totalFood = 0; // reset total food vào đầu ngày
        }
    }
    if (Timer(&time5, waitSpeaker)){ // loa
        if (speakerOn){
            callPet(); // gọi Pet
        }
    }
    if (Timer(&time6, 5000)){ // thông báo
        if (motionNoti){
            publishData("noti", "1");
            motionNoti = false; // Reset
        }else {
            publishData("noti", "0");
        }
        if (sensor[1]){
            publishData("food", "0"); // hơi ngược một tí. do cảm biến. 
        }else{
            publishData("food", "1");
        }
        if (totalFood>0){
            String message = String("SUM ") + String(totalFood);
            publishData("chart", message.c_str()); // thông báo tổng số lượng thức thức ăn cho thú cưng trong ngày
        }
        if (hungry>0){
            hungry--;
        }else{
            if (motionNoti){
                publishData("noti", "2"); // thông báo thú cưng đói
                hungry = 600; // 10 phút sau thông báo lại
            }
        }
    }
    
    
    // Đồng bộ thời gian mỗi 15 phút
    updateTimeSync();
}



// logic:
//    người dùng có thể tự nhấn nút cho ăn hoặc tự động theo lịch đã đặt trước (hẹn giờ cho ăn)
//    chế độ tự động có thể bật tắt, mess ON là bật chế độ tự động, mess OFF là tắt chế độ tự động.
//    khi bật hay tắt chế độ tự động người dùng vẫn hoàn toàn có thể cho ăn được bằng cách gửi mess FEED

//    khi đến giờ cho ăn, hoặc người dùng cho ăn. máy sẽ kêu 5 lần.
//    khi có pet đến, cảm biến chuyển động phát hiện, loa sẽ kêu thêm 1 lần nữa và động cơ quay khiến cho thú cưng có đồ ăn
//    lượng thức ăn được đưa ra theo số lượng đã cài đặt trước (từ 1 đến 10). 
//    khi hết thức ăn, thức ăn sẽ không che được cảm biến hồng ngoại đi nữa. khi đó cảm biến hồng ngoại sẽ phát hiện được hết 
//    thức ăn và gửi thông báo đến người dùng. 
//    khi phát hiện chuyển động thì sẽ có thông báo trên web rằng, có lẽ thú cưng bạn đang đói, có nên cho ăn không.

//    khi cho ăn, sẽ có thông báo lượng thức ăn đã cho
//    sẽ có thông báo tổng lượng thức ăn hàng ngày sau mỗi 5s

//    có 4 bộ hẹn giờ cho ăn, trên web thì dùng có 3. có thể cài đặt bất kể giờ nào trong ngày. nếu giờ hẹn đã qua thì cho ăn ngày hôm sau
//    có thể bật tắt bất kể bộ hẹn giờ nào

