#include "fish-servo.h"

void FishServo::init(int pin) {
    // Allow allocation of all timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    servo.attach(pin);
    currentPos = 0;
}

void FishServo::fullRotation(int delayIn) {
    goToPosition(180);
    delay(delayIn);
    goToPosition(0);
    delay(delayIn);
}

void FishServo::reset() {
    goToPosition(0);
}

void FishServo::goToPosition(int pos) {
    servo.write(pos);
    currentPos = pos;
}
