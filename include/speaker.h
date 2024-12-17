#ifndef SPEAKER_H
#define SPEAKER_H



extern int waitSpeaker;
extern bool speakerOn;


// Khai báo hàm khởi tạo và các hàm cần thiết
void setupSpeaker(int SPEAKER);
void callPet();
void setCalls(int number);
void resetSpeaker();
#endif
