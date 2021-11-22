#ifndef _FISH_SERVO_H_
#define _FISH_SERVO_H_

#include <ESP32Servo.h>

class FishServo {
  private:
    Servo servo;
    int currentPos;


  public:
    /**
     * @brief Constructs a new FishServo object
     * 
     * @param pin The servo PWM signal is sent through this pin number
     */
    void init(int pin);


    /**
     * @brief Rotates the motor to 180 degrees, pauses, rotates the motor to 0 degrees
     * 
     * @param delayIn motor pause in between rotations (in ms)
     */
    void fullRotation(int delayIn);



    /**
     * @brief Resets the servo position to 0 degrees
     */
    void reset();


    /**
     * @brief Rotates the motor to position passed in
     * 
     * @param pos Servo motor position in degrees REQUIRES: 0 <= pos <= 180
     *            
     */
    void goToPosition(int pos);
};

#endif