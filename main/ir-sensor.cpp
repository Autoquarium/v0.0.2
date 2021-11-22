#include "ir-sensor.h"


int IRSensor::readVoltage(){
  int returnVal = analogRead(irPin);
  return returnVal;
}


void IRSensor::init(int irPin_in, int IR_THRESHOLD_in){
  IR_THRESHOLD = IR_THRESHOLD_in;
  irPin = irPin_in;
  pinMode(irPin, INPUT);
}


int IRSensor::getFoodLevel() {
  int irVal = readVoltage();
  Serial.println(irVal);
  if(irVal > IR_THRESHOLD){
    Serial.println("LOW FOOD LEVEL!");
    return 0;
  }
  else{
    Serial.println("Food level is good");
    return 1;
  }
}

