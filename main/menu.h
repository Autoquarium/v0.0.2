
#ifndef _MENU_H_
#define _MENU_H_

#include <Preferences.h>
#include <WiFi.h>


class Menu {

	private:

		// Commands
		const String version = "0.0.1";
		const String get_input = "GETIN"; // get input from the user
		const String get_pass = "GETPS"; // get password input from the user
		const String clear_screen = "CLRSCR"; // clear the entire screen
		const String exit_cli = "EXIT"; // exit the CLI tool

		// value of last user menu selection
		volatile int selection = 0;

		// function pointer to the current menu
		int (Menu::*current_menu)();


		// led pin for wifi indicator
		int led_pin;

		Preferences preferences;


		// FUNCTIONS
		/**
		 * @brief sends start message to the application on user PC, timout after 12 seconds
		 * 
		 * @return int 1: connected to application on computer
		 * 			   2: did not connect to application
		 */
		int send_start();


		/**
		 * @brief Get the Input from the user typing on the PC application
		 * 
		 * @return String the user input
		 */
		String getInput();


		/**
		 * @brief Get the Password that the user is typing on the PC application
		 * 
		 * @return String the user entered password
		 */

		String getPassword();


		/**
		 * @brief Asks the user to make a section of options (1,2,3,ect..)
		 * 
		 * @return int the selection made by the user
		 */
		int makeSelection();


		/**
		 * @brief Admin tool that allows for admin to change local variables and setting on the device
		 * 
		 */
		void adminFunction();


		/**
		 * @brief Tests the connection for a given wifi 
		 * 
		 * @param ssid_in the name of the wifi network to test
		 * @param passwrd_in password to the wifi network to connet to
		 */
		void testWifi(String ssid_in, String passwrd_in);


		/**
		 * @brief draws the admin menu
		 * 
		 * @return int the number associated with the admin menu
		 */
		int adminMenu();


		/**
		 * @brief draws the admin login menu, prompts user for input
		 * 
		 * @return int the number associated with the admin login menu
		 */
		int adminLogin();


		/**
		 * @brief draws the device info menu
		 * 
		 * @return int the number associated with the device info menu
		 */
		int deviceInfoMenu();


		/**
		 * @brief draws the timezone menu and propts for user input
		 * 
		 * @return int the number associated with the time zone menu
		 */
		int timeZoneSetup();


		/**
		 * @brief draws the menu for notification setup, prompts user for input
		 * 
		 * @return int the number associated with the notifcation setup menu 
		 */
		int notiSetup();


		/**
		 * @brief draws the menu for wifi setup, prompts user for inputs
		 * 
		 * @return int the number associated with the wifi setup menu
		 */
		int wifiMenu();


		/**
		 * @brief draws the main menu, prompts user for input
		 *  
		 * @return int the number associated with the main menu 
		 */
		int menu1();


		/**
		 * @brief determines the next menu given the current menu and the user selection
		 * NOTE: this function triggers a reboot of the processor when the menu is quited
		 * 
		 * @param menu_val the number associated with the current menu
		 */
		void execute(int menu_val);

	public:

		/**
		 * @brief Construct a new Menu object
		 * 
		 * @param led_pin_in wifi indicator LED pin number
		 */
		Menu(int led_pin_in) {
			led_pin = led_pin_in;
		}

		/**
		 * @brief loops through all the menus as the user inputs values
		 * 
		 */
		void loop();
};
#endif
