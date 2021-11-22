// About this class:
// FishMqtt combines the functionality of the WiFi client and MQTT client, 
// this allows for more things to be done with less code and clutter

#include "fish-mqtt.h"


void FishMqtt::setWifiCreds(String SSID_in, String PWD_in) {

    if (SSID_in.length() <= 40 && PWD_in.length() <= 40) {
		SSID_in.toCharArray(wifi_SSID, SSID_in.length() + 1);
		PWD_in.toCharArray(wifi_PWD, PWD_in.length() + 1);
    } else {
        Serial.println("[ERROR] Could not set wiFi SSID or password");
    }
}

void FishMqtt::connectToWifi() {
    int status = WL_IDLE_STATUS;
    Serial.print("Connecting to ");

    WiFi.begin(wifi_SSID, wifi_PWD);
    Serial.println(wifi_SSID);
    while (status != WL_CONNECTED) {
        Serial.print(".");
        status = WiFi.begin(wifi_SSID, wifi_PWD);
        delay(10000);
    }
    Serial.println(WiFi.RSSI());
    Serial.println("Connected to WiFi");

    //needed to bypass verification
    // TODO: change to something more secure 
    // (see here: https://github.com/hometinker12/ESP8266MQTTSSL)
    espClient.setInsecure(); 
}


void FishMqtt::MQTTreconnect() {
    Serial.println("Connecting to MQTT Broker...");
    while (!connected()) {
        Serial.println("Reconnecting to MQTT Broker..");
        if (connect(clientName, usrname, password)) {
            Serial.println("Connected to broker.");
            // subscribe to topic
            subscribe("autoq/cmds/#"); //subscribes to all the commands messages triggered by the user
            Serial.println("Subscribed to topic: commands");
            return;
        }
        if(WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFI disconnected");
            connectToWifi();
        }
    }
}


void FishMqtt::checkWificonnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("Connection was lost");
        connectToWifi();
    }
}


void FishMqtt::setupMQTT() {
    setServer(mqttServer, mqttPort);
    delay(1500);
    MQTTreconnect();
}


void FishMqtt::publishSensorVals(float tempVal, float pHVal, int time) {

    // Serialize the sensor data
    DynamicJsonDocument doc(1024);
    doc["pH_val"] = pHVal;
    doc["temp_val"] = tempVal;
    doc["time"] = time;
    String output;
    serializeJson(doc, output);


    // publish the data to the broker
    if (!connected()) MQTTreconnect();
    publish("autoq/sensor/output", output.c_str()); //need to convert to c_string
}


void FishMqtt::setAlertCreds(String User) {
    user_alrt = User;
}


void FishMqtt::sendPushAlert(String msg) {
    Serial.println("sending notifiction");
    HTTPClient http;
    String url = "https://api.pushover.net/1/messages.json";
    String data_to_send = "token=" + API_key + "&user=" + user_alrt + "&message=" + msg;
    Serial.println(data_to_send);

    http.begin(espClient, url);  //Specify destination for HTTP request
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(data_to_send);
    Serial.println(httpResponseCode);
    http.end();  //Free resources
}