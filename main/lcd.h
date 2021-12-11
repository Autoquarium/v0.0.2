/* 
 * https://github.com/adafruit/Adafruit-GFX-Library
 * https://github.com/adafruit/Adafruit_ILI9341
 * https://github.com/adafruit/Adafruit_BusIO 
*/

#ifndef _LCD_H_
#define _LCD_H_

#include <Adafruit_GFX.h> 
#include <Adafruit_ILI9341.h> 
#include  <SPI.h>


class LCD {

	private:
		// LCD properties object
		Adafruit_ILI9341 *tft;

		// Colors for LCD Display
		const int black = 0x0000;
		const int white = 0xFFFF;  // RGB
		const int red = 0xF800;  // R
		const int green = 0x3606;  // G
		const int water_blue = 0x033F6;  // B
		const int yellow  = 0xFFE0;  // RG
		const int cyan  = 0x07FF;  // GB
		const int magenta = 0xF81F;  // RB
		const int gray  = 0x0821;  // 00001 000001 00001
		const int orange = 0xFB46;

		/**
		 * @brief Helper function for printing text to LCD
		 *  
		 * @param text the message to be printed
		 * @param color the color of the message
		 * @param x horizontal position of the message
		 * @param y vertical position of message
		 * @param textSize the size of the text to be displaced on LCD
		 */
		void printText(String text, uint16_t color, int x, int y,int textSize);

	public:

		/**
		 * @brief Construct a new LCD object
		 * 
		 */
		LCD() { }


		/**
		 * @brief init the LCD screen
		 * 
		 * @param TFT_CS pin
		 * @param TFT_DC pin
		 * @param TFT_MOSI master out, slave in pin
		 * @param TFT_CLK clock pin
		 * @param TFT_RST reset pin
		 * @param TFT_MISO multiple input single output pin
		 */
		void init(int TFT_CS, int TFT_DC, int TFT_MOSI, int TFT_CLK, int TFT_RST, int TFT_MISO);


		/**
		 * @brief Updates LCD screen with data
		 * 
		 * @param tempVal Temperature sensor measurement
		 * @param pHVal pH sensor measurement
		 * @param foodLevel Auto-feed module food level status
		 * @param numFish Number of fish in the tank
		 */
		void updateLCD(float tempVal, float pHVal, int foodLevel, int numFish);
};

#endif
