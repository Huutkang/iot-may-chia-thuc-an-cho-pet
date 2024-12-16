#ifndef SENSOR_H
#define SENSOR_H


// extern int SS[2];
extern bool sensor[2];

void setupSensors(int MOTIONSENSOR, int FOODSENSOR);
void readSensors();

#endif
