#include "speaker.h"
#include <Arduino.h>



// Mảng thời gian bật (ON) và tắt (OFF) để tạo giai điệu (đơn vị: milliseconds)
int melody[] = {200, 150, 300, 150, 400, 100, 250, 200, 300, 300}; // Giai điệu có nhịp vui nhộn
int melodyLength = sizeof(melody) / sizeof(melody[0]);
int pinSpeaker;
int index_melody = 0; // Vị trí hiện tại trong mảng melody[]
int waitSpeaker = melody[0]; // thời gian chờ để đến index tiếp theo trong mảng melody. dùng cho hàm Timer ở main.cpp
bool speakerOn = false;

// Hàm khởi tạo chân cho loa
void setupSpeaker(int SPEAKER) {
    pinMode(SPEAKER, OUTPUT); // Thiết lập chân SPEAKER là đầu ra
    digitalWrite(SPEAKER, LOW); // Đảm bảo loa tắt khi khởi tạo
    pinSpeaker = SPEAKER; // Lưu giữ chân loa
}

// Hàm gọi thú cưng ăn với giai điệu
void callPet() {
    if (index_melody&1){
        // index_melody&1 kiểm tra tính chẵn lẽ của biến index_melody
        // bạn có thể dùng if (index_melody%2) hay if (index_melody%2==1) nhưng tốc độ nó sẽ chậm hơn chút
        
        digitalWrite(pinSpeaker, HIGH); // Vị trí lẽ. Bật còi
    } else{
        digitalWrite(pinSpeaker, LOW); // Vị trí lẽ. Tắt còi
    }
    // Di chuyển đến bước tiếp theo trong mảng melody[]
    index_melody++;
    if (index_melody >= melodyLength) {
        index_melody = 0; // Quay lại từ đầu khi hết mảng
    }
    waitSpeaker = melody[index_melody];
}

void resetSpeaker(){
    index_melody = 0;
    waitSpeaker = melody[0];
}
