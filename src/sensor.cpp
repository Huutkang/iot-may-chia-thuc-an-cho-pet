#include "sensor.h"
#include <Arduino.h>


int ss[2];
bool sensor[2] = {false, false};


void setupSensors(int MOTIONSENSOR, int FOODSENSOR) {
    pinMode(MOTIONSENSOR, INPUT);
    pinMode(FOODSENSOR, INPUT);
    ss[0] = MOTIONSENSOR;
    ss[1] = FOODSENSOR;
}

void readSensors() {
    sensor[0] = digitalRead(ss[0]);
    sensor[1] = digitalRead(ss[1]);
}

