#ifndef MOTOR_H
#define MOTOR_H

extern bool motorOn;

void setupMotor(int IN1, int IN2, int IN3, int IN4);
void stepMotor(int step);
void setRotate(float revolutions);
void rotateMotor();

#endif
