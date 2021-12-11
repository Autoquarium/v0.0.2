// About this class:
// FishMqtt combines the functionality of the WiFi client and MQTT client, 
// this allows for more things to be done with less code and clutter

#ifndef _FISH_MQTT_H_
#define _FISH_MQTT_H_

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

class FishMqtt : public PubSubClient {
	private:


		// device info
		String device_id = "autoq-prod"; // default device

		// MQTT broker info
		char *mqttServer =  "e948e5ec3f1b48708ce7748bdabab96e.s1.eu.hivemq.cloud"; // test
		int mqttPort = 8883;

		// MQTT client credentials
		char *clientName = "FishClient-666";
		char *usrname = "fishusr";
		char *password = "Fish123!";

		// WiFi credentials
		char wifi_SSID[40];
		char wifi_PWD[40];
		
		// Alert credentials
		HTTPClient http;
		String user_alrt;
		String API_key = "akiafy9jms26ojnx53bw5vvivj1s4v";
		
		// WiFi client for esp32 chip
		WiFiClientSecure espClient;

		// indicator LED light
		int led_pin;


	public:

		/**
		 * @brief Construct a new Fish Mqtt object
		 * 
		 */
		FishMqtt() : PubSubClient(espClient) {}


		/**
		 * @brief setup and turn on wifi indicator led
		 * 
		 * @param wifiLedPin the pin connected to the front light
		 */
		void ledSetup(int wifiLedPin);

		/**
		 * @brief Set the Device Id to be sent in the published message
		 * 
		 * @param device_id_in the device id to be set
		 */
		void setDeviceId(String device_id_in);


		/**
		 * @brief sets the wifi SSID and password, dose not begin wifi connection 
		 * 
		 * @param SSID_in The name of the WiFi network to connect to
		 * @param PWD_in The password of the WiFi network to connect to
		 */
		void setWifiCreds(String SSID_in, String PWD_in);


		/**
		 * @brief Connects to the WiFi using the credentials provided in the class variables
		 *	Loops until connection is established
		* 
		*/
		void connectToWifi();



		/**
		 * @brief Connects the MQTT broker specified in the setServer() call.
		 * If needed, reconnects to the configured WiFi using connectToWifi()
		 *
		 */
		void MQTTreconnect();


		/**
		 * @brief Checks wifi connection
		 * 
		 * @return true wifi is still connected
		 * @return false wifi is no longer connected
		 */
		bool checkWificonnection();


		/**
		 * @brief sets the server and calls MQTTrecconnect() to connect to WiFi and the broker
		 *
		 */
		void setupMQTT();


		/**
		 * @brief This function serializes the inputted values and then 
		 * publishes the serialized string to the MQTT broker
		 * 
		 * @param tempVal the temperature value to be published
		 * @param pHVal the pH value to be published
		 * @param time the time that the data was published
		 */
		void publishSensorVals(float tempVal, float pHVal, int time);

		
		/**
		 * @brief This function serializes food level and then 
		 * publishes the serialized string to the MQTT broker
		 * 
		 * @param foodLevel true if food remains, false otherwise
		 */
		void publishFoodLevel(bool foodLevel);


		/**
		 * @brief Set the Alert Credentials
		 * 
		 * @param User API user
		 */
		void setAlertCreds(String User);


		/**
		 * @brief Sends a notification to user
		 * 
		 * @param msg message to be included in the notification
		 */
		void sendPushAlert(String msg);
};

#endif
