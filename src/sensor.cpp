#include "sensor.h"
#include <Arduino.h>


int SS[2];
bool sensor[2] = {false, false};


void setupSensors(int MOTIONSENSOR, int FOODSENSOR) {
    pinMode(MOTIONSENSOR, INPUT);
    pinMode(FOODSENSOR, INPUT);
    SS[0] = MOTIONSENSOR;
    SS[1] = FOODSENSOR;
}

void readSensors() {
    sensor[0] = digitalRead(SS[0]);
    sensor[1] = digitalRead(SS[1]);
}

