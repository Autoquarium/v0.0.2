
#ifndef _LED_ARRAY_H_
#define _LED_ARRAY_H_


#include <FastLED.h>

class LEDArray {
  private:

    CRGB LEDs[300];
    int numLEDs;
    CRGB currentRGB;

    const CRGB sunrise = CRGB(255,167,0);
    const CRGB day = CRGB(100,100,100);
    const CRGB sunset = CRGB(245,123,37);
    const CRGB night = CRGB(0,0,40);

    // dynamic lighting transition schedule
    const double sunriseTStart = 400;
    const double sunriseTEnd = 559;
    const double dayTStart = 600;
    const double dayTEnd = 1000;
    const double sunsetTStart = 1400;
    const double sunsetTEnd = 1759;
    const double nightTStart = 1800;
    const double nightTEnd = 2200;

    /**
     * @brief Turns off the LEDs by "fading to black"
     * 
     * @param delayIn Time it will take the LEDs to fade to black in milliseconds
     *                Affects the smoothness of the transition
     */
    void fadeToBlack(int delayIn);


    /**
     * @brief Sets the hue/saturation/value of the lights
     *        Would prefer if this function was not used (does not update currentRGB)
     * 
     * @param h hue (color range shown in FastLED documentation) REQUIRES: 0 <= h <= 255
     * @param s saturation REQUIRES: 0 <= h <= 255
     * @param v brightness REQUIRES: 0 <= h <= 255
     */
    void setHSVColor(int h, int s, int v);


    /**
     * @brief Sets the RGB color of the lights
     * 
     * @param r red value intensity REQUIRES: 0 <= r <= 255
     * @param g green value intensity REQUIRES: 0 <= g <= 255
     * @param b blue value intensity REQUIRES: 0 <= b <= 255
     */
    void setRGBColor(int r, int g, int b);


    /**
     * @brief Sets the RGB color of the lights
     * 
     * @param color is of type CRGB from FastLED
     */
    void setRGBColor(CRGB color);



    /**
     * @brief Transitions the color of the LEDs from RGB1 to RGB2
     * 
     * @param r1 REQUIRES: 0 <= r1 <= 255
     * @param g1 REQUIRES: 0 <= g1 <= 255
     * @param b1 REQUIRES: 0 <= b1 <= 255
     * @param r2 REQUIRES: 0 <= r2 <= 255
     * @param g2 REQUIRES: 0 <= g2 <= 255
     * @param b2 REQUIRES: 0 <= b2 <= 255
     * @param delayIn Time it will take the LEDs to fade from RGB1 to RGB2 in milliseconds
     *                Affects the smoothness of the transition
     */
    void colorTransition(int r1, int g1, int b1, int r2, int g2, int b2, int delayIn);


    /**
     * @brief Transitions the color of the LEDs from RGB1 to RGB2
     * 
     * @param color1 LEDs change to this color when called
     * @param color2 LEDs fade to this color over time delayIn
     * @param delayIn Time it will take the LEDs to fade from RGB1 to RGB2 in milliseconds
     *                Affects the smoothness of the transition
     */
    void colorTransition(CRGB color1, CRGB color2, int delayIn);


    /**
     * @brief Transitions from startColorVal to endColorVal, holds endColorVal for delayIn, 
     *        then transitions back to startColorVal
     * 
     * @param startColorVal REQUIRES: 0 <= startColorVal[i] <= 255
     * @param endColorVal REQUIRES: 0 <= endColorVal[i] <= 255
     * @param delayIn Time that the LEDs will hold endColorVal in milliseconds
     */
    void createColorCycle(int startColorVal[], int endColorVal[], int delayIn);



  public:

  
    /**
     * @brief Constructs a new LEDArray object
     *        Initializes pins and FastLED variables
     * 
     * @param numLEDsIn Number of lights of the strip to be turned on REQUIRES: numLEDsIn <= 300
     * @param ledPinIn IO pin that data will be sent across to control the LEDs
     */
    void init(const int ledPinIn, int numLEDsIn);



    /**
     * @brief Slowly transitions from current color to new color
     * 
     * @param r REQUIRES: 0 <= r1 <= 255
     * @param g REQUIRES: 0 <= r1 <= 255
     * @param b REQUIRES: 0 <= r1 <= 255
     */
    void changeColor(int r, int g, int b);




    /**
     * @brief Updates color blend based on the time of day
     *        Mimics a sunrise-day-sunset-night cycle
     *
     *        TIME: COLOR
     *        0400: night, start transition to sunrise
     *        0600: sunrise
     *        1000: day
     *        1400: day, start transition to sunset
     *        1800: sunset
     *        2200: night
     * 
     * @param currentTime REQUIRES: 0 <= currentTime < 2400
     */
    void updateDynamicColor(int currentTime);

};

#endif
