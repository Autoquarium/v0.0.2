#ifndef _IR_SENSOR_H_
#define _IR_SENSOR_H_

#include "Arduino.h"

class IRSensor {   
  private:
    int irPin;
    int ledPin;
    int IR_THRESHOLD;

    /*Sets LED pin on, reads voltage from ADC, sets LED pin off*/
    int readVoltage();

  public:
    /**
     * @brief Construct a new ir sensor object
     * 
     */
    IRSensor() {}

    
    /**
     * @brief Initialize analog pin for use with IR Sensor
     * 
     * @param irPin_in pin the IR photodiode is connected to
     * @param IR_THRESHOLD_in cutoff for object detection
     */
    void init(int irPin_in, int IR_THRESHOLD_in);


    /**
     * @brief Get the Food Level
     * 
     * @return int, 1 if full, 0 otherwise
     */
    int getFoodLevel();
};
#endif