#include "motor.h"
#include <Arduino.h>

// Số bước trong một chu kỳ
const int stepsPerRevolution = 4096; // 64 bước x tỷ số truyền 1:64

// Trình tự điều khiển động cơ bước (half-step)
int stepSequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1},
};

// mảng lưu các chân điều khiển motor
int MT[4];
bool motorOn = false;
// Biến trạng thái
int currentStep = 0;
// tổng số bước cần phải thực hiên tiếp
int totalSteps = 0;


void setupMotor(int IN1, int IN2, int IN3, int IN4) {
    MT[0] = IN1;
    MT[1] = IN2;
    MT[2] = IN3;
    MT[3] = IN4;
    for (int i = 0; i < 4; i++) {
        pinMode(MT[i], OUTPUT);
        digitalWrite(MT[i], LOW); // Tắt motor ban đầu
    }
}

// Hàm điều khiển một bước
void stepMotor(int step) {
    digitalWrite(MT[0], stepSequence[step][0]);
    digitalWrite(MT[1], stepSequence[step][1]);
    digitalWrite(MT[2], stepSequence[step][2]);
    digitalWrite(MT[3], stepSequence[step][3]);
}

// Đặt góc quay cho motor để motor hoạt động. nó tỉ lệ với lượng thức ăn
void setRotate(float revolutions){
    totalSteps = (int)(revolutions * stepsPerRevolution); // Tính tổng số bước từ số vòng quay
}

// Hàm quay động cơ theo số vòng quay đã đặt
void rotateMotor() {
    if (totalSteps>0) {
        currentStep = (currentStep + 1) % 8; // Tiến
        stepMotor(currentStep);
        totalSteps--;
    }else{
        motorOn = false;
        totalSteps = 0;
        for (int i = 0; i < 4; i++) {
            digitalWrite(MT[i], LOW); // Tắt motor
        }
    }
}
