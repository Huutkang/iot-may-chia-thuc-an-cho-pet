#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <Arduino.h>


extern bool feeding_timer;


// Khai báo hàm khởi tạo và các hàm cần thiết
void setupTimeSync();
void updateTimeSync();
String getCurrentTime(); // Lấy thời gian trong ngày dưới dạng HH:MM:SS
unsigned long getSecondsSinceMidnight(); // Lấy số giây từ đầu ngày
void initializeTimers();
void SetWateringTimer(int pumpIndex, int timerIndex, unsigned long startTimeInSeconds);
void ProcessTimerString(String& input);
void checkAndActivateTimers();

#endif
