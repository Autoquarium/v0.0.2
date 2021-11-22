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
		// MQTT broker info
		char *mqttServer =  "e948e5ec3f1b48708ce7748bdabab96e.s1.eu.hivemq.cloud";
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


	public:

		/**
		 * @brief Construct a new Fish Mqtt object
		 * 
		 */
		FishMqtt() : PubSubClient(espClient) {}


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
		 * @brief Checks wifi connection and reconnects if needed
		 * 
		 */
		void checkWificonnection();


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
		 * @brief Set the Alert Credentials
		 * 
		 * @param User API user
		 */
		void setAlertCreds(String User);


		// "https://api.pushover.net/1/messages.json?token=akiafy9jms26ojnx53bw5vvivj1s4v&user=uaeiijpxfayt5grxg85w97wkeu7gxq&message=testing";
		/**
		 * @brief Sends a notification to user
		 * 
		 * @param msg message to be included in the notification
		 */
		void sendPushAlert(String msg);
};

#endif