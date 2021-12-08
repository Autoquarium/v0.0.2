// About this class:
// FishMqtt combines the functionality of the WiFi client and MQTT client, 
// this allows for more things to be done with less code and clutter

#include "fish-mqtt.h"


void FishMqtt::ledSetup(int wifiLedPin) {
    led_pin = wifiLedPin;
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, HIGH);
}

void FishMqtt::setDeviceId(String device_id_in) {
    device_id = device_id_in;
}

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
        digitalWrite(led_pin, LOW);
        delay(1500);
        digitalWrite(led_pin, HIGH);
        delay(1500);
        digitalWrite(led_pin, LOW);
        delay(1500);
        digitalWrite(led_pin, HIGH);
        delay(1500);
        digitalWrite(led_pin, LOW);
        delay(1500);
        digitalWrite(led_pin, HIGH);
        delay(1500);
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
            //Serial.println("Connected to broker.");
            //subscribe to topic
            String topic_str = device_id + "/cmds/#";
            char topic_c[40];
            topic_str.toCharArray(topic_c, topic_str.length() + 1);
            subscribe(topic_c); //subscribes to all the commands messages triggered by the user
            Serial.print("Subscribed to topic: ");
            Serial.println(topic_c);
            return;
        }
        if(WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFI disconnected");
            connectToWifi();
        }
        setKeepAlive( 90 ); 
    }
}


bool FishMqtt::checkWificonnection() {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }
    return true;
}


void FishMqtt::setupMQTT() {
    setKeepAlive( 90 ); 
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
    doc["device_id"] = device_id;
    String output;
    serializeJson(doc, output);
    
    // publish the data to the broker
    if (!connected()) MQTTreconnect();
    publish("autoq/sensor/output", output.c_str()); //need to convert to c_string
}



void FishMqtt::publishFoodLevel(bool foodLevel) {
    DynamicJsonDocument doc(1024);
    doc["food_remain"] = foodLevel;
    doc["device_id"] = device_id;
    String output;
    serializeJson(doc, output);
    // example output:
    // {"food_remain": true, "device_id": "123"}


    // publish the data to the broker
    if (!connected()) MQTTreconnect();
    publish("autoq/sensor/feed", output.c_str()); //need to convert to c_string
    
    if (!foodLevel) {
      sendPushAlert("Fish have been fed", "Fish Food Level is Low!");
    } else {
      sendPushAlert("Fish have been fed");
    }
    MQTTreconnect();
}

void FishMqtt::setAlertCreds(String User) {
    user_alrt = User;
}

void FishMqtt::sendPushAlert(String msg1, String msg2) {
  
  HTTPClient http;
  String url = "https://api.pushover.net/1/messages.json";
  http.begin(espClient, url);  //Specify destination for HTTP request
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // post first message
  if (msg1.length() != 0) {
    String data_to_send = "token=" + API_key + "&user=" + user_alrt + "&message=" + msg1;
    http.POST(data_to_send);
  }

  // post second message
  if (msg2.length() != 0) {
    String data_to_send = "token=" + API_key + "&user=" + user_alrt + "&message=" + msg2;
    http.POST(data_to_send);
  }
  
  //Free resources
  http.end();
}

void FishMqtt::sendPushAlert(String msg) {
    Serial.println("sending notifiction");
    try {
        HTTPClient http;
        String url = "https://api.pushover.net/1/messages.json";
        String data_to_send = "token=" + API_key + "&user=" + user_alrt + "&message=" + msg;
        http.begin(espClient, url);  //Specify destination for HTTP request
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        int httpResponseCode = http.POST(data_to_send);
        Serial.print("HTTP response: ");
        Serial.println(httpResponseCode);
    }
    catch(...) {
        Serial.println("ERROR SENDING NOTIFICATION");
    }
    http.end();  //Free resources
}
