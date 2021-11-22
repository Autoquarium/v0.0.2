#ifndef _TEMP_SENSOR_H_
#define _TEMP_SENSOR_H_

#include <OneWire.h> 

class TempSensor {
  
  private:
    OneWire *ds;

  public:
    /**
     * @brief Construct a new Temp Sensor object
     * 
     */
    TempSensor() { }
  
    /**
      * @brief initializes the OneWire object 
      * 
      * @param pin Sets the pin that sensor reads from
      */
    void init(int pin);
  

    /**
      * @brief Get the current reading from the temperature sensor
      * 
      * @return float value of the temperature in Celsius 
      */
    float getTemp();
};

#endif