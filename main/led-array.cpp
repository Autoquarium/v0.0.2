#include "led-array.h"

void LEDArray::fadeToBlack(int delayIn) {
    int itrs = 20;
    int itrDelay = (double)delayIn/(double)itrs;
    while (itrs > 0) {
        // half the brightness of all LEDs until dark
        fadeToBlackBy(LEDs, numLEDs, 64);
        FastLED.show();
        delay(itrDelay);
        --itrs;
    }
}


void LEDArray::setHSVColor(int h, int s, int v) {
    for (int i = 0; i < numLEDs; ++i) {
        LEDs[i] = CHSV(h,s,v);
    }
    FastLED.show();
}


void LEDArray::setRGBColor(int r, int g, int b) {
    setRGBColor(CRGB(r,g,b));
}



void LEDArray::setRGBColor(CRGB color) {
    for (int i = 0; i < numLEDs; ++i) {
        LEDs[i] = color;
    }
    FastLED.show();
    currentRGB = color;
}



void LEDArray::colorTransition(int r1, int g1, int b1, int r2, int g2, int b2, int delayIn) {
    colorTransition(CRGB(r1, g1, b1), CRGB(r2, g2, b2), delayIn);
}


void LEDArray::colorTransition(CRGB color1, CRGB color2, int delayIn) {
    int itrDelay = delayIn/(255/5);
    for (int percent = 0; percent <= 255; percent += 5) {
        setRGBColor(blend(color1, color2, percent));
        delay(itrDelay);
    }
}


void LEDArray::createColorCycle(int startColorVal[], int endColorVal[], int delayIn) {
    int transitionDelay = 2000; // 2 seconds
    colorTransition(startColorVal[0], startColorVal[1], startColorVal[2], 
                endColorVal[0], endColorVal[1], endColorVal[2], transitionDelay);
    delay(delayIn);
    colorTransition(endColorVal[0], endColorVal[1], endColorVal[2], 
                startColorVal[0], startColorVal[1], startColorVal[2], transitionDelay);
}


void LEDArray::init(const int ledPinIn, int numLEDsIn) {
    numLEDs = numLEDsIn;
    switch (ledPinIn) {
        case 22:
            FastLED.addLeds<NEOPIXEL, 22>(LEDs, numLEDs);
            break;

        case 23:
            FastLED.addLeds<NEOPIXEL, 23>(LEDs, numLEDs);
            break;

        case 2:
            FastLED.addLeds<NEOPIXEL, 2>(LEDs, numLEDs);
            break;

        default:
            Serial.println("Unsupported Pin");
            break;
    }

    setRGBColor(255, 255, 255);
}

void LEDArray::changeColor(int r, int g, int b) {
    colorTransition(currentRGB, CRGB(r, g, b), 5000);
}

void LEDArray::updateDynamicColor(int currentTime) {

    CRGB color1, color2;
    double dynamicBlendPercent;

    if (currentTime < sunriseTStart) { /* NIGHT */
        colorTransition(currentRGB, night, 5000);
        return;
    }
    else if (currentTime <= sunriseTEnd) { /* NIGHT -> SUNRISE */
        dynamicBlendPercent = ((double)currentTime - sunriseTStart) * 255.0 / (sunriseTEnd - sunriseTStart);
        color1 = night;
        color2 = sunrise;
    }
    else if (currentTime <= dayTEnd) { /* SUNRISE -> DAY */
        dynamicBlendPercent = ((double)currentTime - dayTStart) * 255.0 / (dayTEnd - dayTStart);
        color1 = sunrise;
        color2 = day;
    }
    else if (currentTime < sunsetTStart) { /* DAY */
        colorTransition(currentRGB, day, 5000);
        return;
    }
    else if (currentTime < sunsetTEnd) { /* DAY -> SUNSET */
        dynamicBlendPercent = ((double)currentTime - sunsetTStart) * 255.0 / (sunsetTEnd - sunsetTStart);
        color1 = day;
        color2 = sunset;
    }
    else if (currentTime < nightTEnd) { /* SUNSET -> NIGHT */
        dynamicBlendPercent = ((double)currentTime - nightTStart) * 255.0 / (nightTEnd - nightTStart);
        color1 = sunset;
        color2 = night;
    }
    else { /* NIGHT */
        colorTransition(currentRGB, night, 5000);
        return;
    }
    colorTransition(currentRGB, blend(color1, color2, dynamicBlendPercent), 5000);
}
