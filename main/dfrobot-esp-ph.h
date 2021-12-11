/*
 * file dfrobot-esp-ph.h * @ https://github.com/GreenPonik/DFRobotESPpH_BY_GREENPONIK
 *
 * Arduino library for Gravity: Analog pH Sensor / Meter Kit V2, SKU: SEN0161-V2
 * 
 * Based on the @ https://github.com/DFRobot/DFRobot_PH
 * Copyright   [DFRobot](http://www.dfrobot.com), 2018
 * Copyright   GNU Lesser General Public License
 *
 * ##################################################
 * ##################################################
 * ########## Fork on github by GreenPonik ##########
 * ############# ONLY ESP COMPATIBLE ################
 * ##################################################
 * ##################################################
 * 
 * version  V1.0
 * date  2019-05
 */

#ifndef _DFROBOT_ESP_PH_H_
#define _DFROBOT_ESP_PH_H_

#include "Arduino.h"
#include <Preferences.h>

// length of the Serial CMD buffer
#define ReceivedBufferLength 10 


// the start address of the pH calibration parameters stored in the EEPROM
#define PH_8_VOLTAGE 1122
#define PH_6_VOLTAGE 1478
#define PH_5_VOLTAGE 1654
#define PH_3_VOLTAGE 2010


class DFRobotESPpH {
private:
    Preferences preferences; 
    float _phValue;
    float _acidVoltage;
    float _neutralVoltage;
    float _voltage;
    float _temperature;
    
    // ESP32 pin and voltage values
    float ESPADC;
    int ESPVOLTAGE;
    int PH_PIN;

     //store the Serial CMD
    char _cmdReceivedBuffer[ReceivedBufferLength];
    byte _cmdReceivedBufferIndex;

    /**
     * @brief checks to see if serial data is available
     * 
     * @return boolean true if serial data is available, false otherwise
     */
    boolean cmdSerialDataAvailable();
    
    /**
     * @brief calibration process, writes key parameters to EEPROM
     * 
     * @param mode the calibration mode inputted by the user
     */
    void phCalibration(byte mode);
    
    /**
     * @brief parses incoming serial command
     * 
     * @param cmd the serial command entered by user
     * @return byte the parsed command
     */
    byte cmdParse(const char *cmd);
    
    /**
     * @brief parses incoming serial command
     * 
     * @return byte the parsed command
     */
    byte cmdParse();
  
public:
    /**
     * @brief Construct a new DFRobotESPpH object
     * 
     */
    DFRobotESPpH();

    /**
     * @brief Destroy the DFRobotESPpH object
     * 
     */
    ~DFRobotESPpH();

   /**
     * @brief Calibrate the calibration data
     *
     * @param cmd         : ENTERPH -> enter the PH calibration mode
     *                      CALPH   -> calibrate with the standard buffer solution, two buffer solutions(4.0 and 7.0) will be automaticlly recognized
     *                      EXITPH  -> save the calibrated parameters and exit from PH calibration mode
     */
    void calibration(char *cmd); //calibration by Serial CMD
    
    /**
     * @brief Runs calibration sequence for pH sensor
     *        If correct command is received, enters calibration mode
     */
    void calibration();
    
    /**
     * @brief Runs manual calibration sequence for pH sensor
     *        If correct command is received, enters manual calibration mode.
     *        MANCALPH -> enter the PH manual calibration mode
     *        EXIT -> Exit without saving
     *        Calibration uses while loops to wait for incoming data, so it may cause other processes to misbehave.
     */

    void manualCalibration();
    /**
     * @brief Runs calibration sequence for pH sensor
     *        If correct command is received, enters calibration mode
     * 
     * @param voltage7 Voltage value of pH sensor when submerged in pH 7 buffer solution
     * @param voltage4 Voltage value of pH sensor when submerged in pH 4 buffer solution
     */
    void manualCalibration(float voltage7, float voltage4);

    /**
     * @brief Converts voltage read by the pH sensor into pH value
     *        Uses temperature measurement for a more accurate conversion
     * 
     * @param voltage Voltage value of pH sensor
     * @param temperature Temperature in degress celcius
     */
    float readPH(float voltage, float temperature); // voltage to pH value, with temperature compensation

    /**
     * @brief begins the pH sensor reading given the values set in the init function
     * 
     * NOTE: init() should be called prior to evoking this function
     */
    void begin();

    /**
     * @brief Gets the neutral voltage of tank/solution
     * 
     * @return float the neutral voltage value
     */
    float get_neutralVoltage();

    /**
     * @brief Initializes the pH sensor/hardware and assigns it the proper pins
     * 
     * @param PH_PIN_in Input for pH sensor on ESP32
     * @param ESPADC_in Input for ADC from ESP32
     * @param ESPVOLTAGE_in Input for ESP32 Voltage source
     */
    void init(int PH_PIN_in, float ESPADC_in, int ESPVOLTAGE_in);
    
    /**
     * @brief Retrieves the pH value of the tank/solution
     * 
     * @param temp_in Temperature of tank(in Celsius?)
     * @return float
     */
    float getPH(float temp_in);
};

#endif
