#include "lcd.h"

void LCD::printText(String text, uint16_t color, int x, int y,int textSize) {
    tft->setCursor(x, y);
    tft->setTextSize(textSize);
    tft->setTextWrap(true);
    tft->setTextColor(color);
    tft->print(text);
}


void LCD::init(int TFT_CS, int TFT_DC, int TFT_MOSI, int TFT_CLK, int TFT_RST, int TFT_MISO) {
    tft = new Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
    tft->begin();                      
    tft->setRotation(0);            
    tft->fillScreen(ILI9341_BLACK);

    printText("AUTOQUARIUM", water_blue,20,20,3);
    printText("pH", white,25,90,3);
    printText("Temp", white,150,90,3);
    printText("Food", white,25,180,3);
    printText("Fish", white,150,180,3);
}


void LCD::updateLCD(float tempVal, float pHVal, int foodLevel, int numFish) {
    tft->fillScreen(ILI9341_BLACK);
    printText("AUTOQUARIUM", water_blue,20,20,3);
    printText("pH", white,25,90,3);
    printText("Temp", white,150,90,3);
    printText("Food", white,25,180,3);
    printText("Fish", white,150,180,3);

    if(pHVal == 7) {
        printText((String)pHVal, green, 30, 120, 3);
    }
    else if(pHVal < 6.5 || pHVal > 7.5) {
        printText((String)pHVal, red, 30, 120, 3);
    }
    else {
        printText((String)pHVal, orange, 30, 120, 3);
    }

    if(tempVal < 23 || tempVal > 27) {
        printText((String)tempVal, red,160,120,3);
    }
    else {
        printText((String)tempVal, green,160,120,3);
    }

    // food value
    if(foodLevel == 1) {
        printText("Good", green,30,210,3);
    }
    else {
        printText("Low", red,30,210,3);
    }

    // Num Fish
    printText((String)numFish, green,160,210,3);
}
