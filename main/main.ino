#include <Arduino.h>
#include <Preferences.h>

// TODO: change these with new names
#include "menu.h"              /* cli and firsttime setup */
#include "dfrobot-esp-ph.h"    /* pH sensor lib */
#include "ir-sensor.h"         /* low food reading */
#include "fish-servo.h"        /* moving servo */
#include "led-array.h"         /* led lighting control */ 
#include "lcd.h"               /* LCD screen control */
#include "fish-mqtt.h"         /* WiFi, MQTT, and Notifications */
#include "temp-sensor.h"       /* water temp sensor lib */


// semaphores for subscribed MQTT cmds - USE BINARY SEMAPHORES BECAUSE WE ARE TRIGGERING ANOTHER TASK TO RUN
SemaphoreHandle_t feed_semaphore;
SemaphoreHandle_t led_semaphore;
SemaphoreHandle_t setting_semaphore;

// mutex for CMD_PAYLOAD - USE MUTEX BECAUSE ITS A SHARED RESOURCE:
// https://stackoverflow.com/questions/62814/difference-between-binary-semaphore-and-mutex
SemaphoreHandle_t payload_mutex;

// setting variables - loaded from memory / set in CLI or web app
long gmtOffset_sec;
int num_of_fish;
bool dynamic_lighting;
bool send_alert;

// Danger cutoof values
const int MAX_TEMP  = 90;
const int MIN_TEMP = 70;
const int MAX_PH = 9;
const int MIN_PH = 4;

// pH sensor
const float ESPADC = 4096.0;   //the esp Analog Digital Convertion value
const int ESPVOLTAGE = 3300; //the esp voltage supply value
const int PH_PIN = 35;    //pH sensor gpio pin
DFRobotESPpH ph;

// LCD pins
const int TFT_DC = 17;
const int TFT_CS = 15;
const int TFT_RST = 5;
const int TFT_MISO = 19;         
const int TFT_MOSI = 23;           
const int TFT_CLK = 18; 
LCD lcd;
TempSensor temperature;

// ir sensor
const int IR_PIN = 34; //TODO change to ESP pins
const int LED_PIN = 26; //TODO change to ESP pins
const int IR_THRESHOLD = 50; //TODO change to reflect values in enclosure
IRSensor ir;

//Temperature chip
int DS18S20_Pin = 4; //DS18S20 Signal pin on digital 2

// servo
const int SERVO_PIN = 32;
const int DELAY_BETWEEN_ROTATION = 1000;
const int MIN_FEED_INTERVAL = 1200;
FishServo si;
int previous_feed_time = -1;

// LED array
const int ledPin = 1;
const int ledNum = 50;
LEDArray leds;

// MQTT, WiFi and notification
FishMqtt wiqtt;
int publish_interval; // in minutes
char CMD_PAYLOAD[30];

//function prototypes
void userSetup();
void load_settings();
void dangerValueCheck( float tempVal, float pHVal, int foodLevel );


// task handlers
TaskHandle_t dynamicLEDTask;


// MAIN TASKS
void keepWifiConnected( void * parameter ){
  // keep track of last wake
  portTickType xLastWakeTime;

  // set delay period (50 seconds)
  portTickType xPeriod = ( 50000 / portTICK_RATE_MS );
  xLastWakeTime = xTaskGetTickCount ();
  
  for(;;){
    vTaskDelayUntil( &xLastWakeTime, xPeriod ); // run every 5 seconds
    Serial.println("checking wifi connection");
    wiqtt.checkWificonnection();
  }
}

void checkIncomingCmds( void * parameter ){
  // keep track of last wake
  portTickType xLastWakeTime;

  // set delay period (4 seconds)
  portTickType xPeriod = ( 4000 / portTICK_RATE_MS );
  xLastWakeTime = xTaskGetTickCount ();
  
  for(;;){
    vTaskDelayUntil( &xLastWakeTime, xPeriod ); // run every 5 seconds
    Serial.println("calling wiqtt.loop()");
    wiqtt.loop(); // triggers callback if needed
  } 
}


void publishSensorVals( void * parameter ) {
  portTickType xLastWakeTime;

  // convert publish interval from minutes into ms
  int delay_in_ms = publish_interval * 60 * 1000;
  portTickType xPeriod = ( delay_in_ms / portTICK_RATE_MS );
  xLastWakeTime = xTaskGetTickCount ();
  
  for( ;; ) {
    // Wait for the next cycle.
    vTaskDelayUntil( &xLastWakeTime, xPeriod );
    Serial.println("Publishing new sensor values to broker");

    // get water Temp
    float temp_read = 80.1;

    // get pH value
    float pH_read = 7.65;

    // publish data
    wiqtt.publishSensorVals(temp_read, pH_read, 900); // add time
  }
}


void dynamicLightingChange( void * parameter ) {
  portTickType xLastWakeTime;
  
  int delay_in_ms = 10* 60 * 1000; // 10 minutes to ms
  portTickType xPeriod = ( delay_in_ms / portTICK_RATE_MS );
  xLastWakeTime = xTaskGetTickCount ();

  for( ;; ) {

    Serial.println("Dynamic lighting change");

    // getTime
    // leds.updateDynamicColor(time)

    vTaskDelayUntil( &xLastWakeTime, xPeriod );
    }


}


// CMD TASKS - these tasks are triggered inside the callback function
void feedCmdTask( void *pvParameters ) {
  for ( ;; ) {
    xSemaphoreTake(feed_semaphore, portMAX_DELAY);
    Serial.println("feed the fish");
    // call servo function (once every 12 hours max)
//      if((previous_feed_time == -1) || (getTimeDiff(getTime(), previous_feed_time) > MIN_FEED_INTERVAL)){
//        for(int i = 0; i < num_of_fish; i++) {
//          si.fullRotation(1000); // TODO: make this better
//        }
//        previous_feed_time = getTime();
//      }
//      else{
//        Serial.println("Unable to feed, time interval too close.");
//      }
  }
}


void ledCmdTask( void *pvParameters ) {
  for ( ;; ) {
    xSemaphoreTake(led_semaphore, portMAX_DELAY);
    Serial.println("change led color");
//    Serial.println("change led color");   
//      int r = atoi(strtok(buff, ","));
//      int g = atoi(strtok(NULL, ","));
//      int b = atoi(strtok(NULL, ","));
//      leds.changeColor(r, g, b);
  }
}


void settingCmdTask( void *pvParameters ) {
  for ( ;; ) {
    xSemaphoreTake(setting_semaphore, portMAX_DELAY);
    Serial.println("change application settings");
    // TODO: add code to change settings


    // if enable dynamic lighting:
    // vTaskResume( dynamicLEDTask )

    // Save new setting values back to memory
    /*
      preferences.begin("saved-values", false);
      publish_interval = preferences.getInt("publish_interval", 2);
      num_of_fish = preferences.getInt("num_of_fish", 3);
      dynamic_lighting = preferences.getBool("dynamic_lighting", false);
      send_alert = preferences.getBool("send_alert", false);
      preferences.end();
    */
  }
}


/**
 * @brief Called everytime a topic that we are subscribed to publishes data, 
 * calls the appropriate functions to perform the needed actions
 * 
 * @param topic the topic that we are subscribed to 
 * @param payload the actual data receaved
 * @param length the legnth of the data receaved
 */
void callback(char* topic, byte* payload, unsigned int length) {
  
    // save payload to CMD_PAYLOAD
    // need to use a mutex for the payload so that the cmd tasks are not reading while this is writting
    int i = 0;
    xSemaphoreTake(payload_mutex, portMAX_DELAY);
    for (; i < length; i++) {
      CMD_PAYLOAD[i] = (char) payload[i];
    }
    CMD_PAYLOAD[i] = '\0';
    xSemaphoreGive(payload_mutex);
    
    // FEEDING CMDS
    if (!strcmp(topic, "autoq/cmds/feed")) {
      xSemaphoreGive(feed_semaphore);
    }

    // LIGHTING CMDS
    else if (!strcmp(topic, "autoq/cmds/leds")) {
      xSemaphoreGive(led_semaphore);
    }

    // SETTING CHANGES
    else if (!strcmp(topic, "autoq/cmds/settings")) {
      xSemaphoreGive(setting_semaphore); 
    }
    
    else {
        Serial.println("Not a valid topic");
    }
   
    return;
}


void taskCreation() {
  xTaskCreatePinnedToCore(
    publishSensorVals,
    "publish sensor vals",
    10000,
    NULL,
    2, // this task is NOT vital for correct system operation
    NULL,
    1
    );                             

  xTaskCreatePinnedToCore(
    keepWifiConnected,
    "keep wifi connected",
    10000,
    NULL,
    3, // this task is VERY vital for correct system operation
    NULL,
    1
    );                             

  xTaskCreatePinnedToCore(
    checkIncomingCmds,
    "check Incoming MQTT msgs",
    10000,
    NULL,
    2, // this task is vital for correct system operation
    NULL,
    1
    );                             

  xTaskCreatePinnedToCore(
    feedCmdTask,
    "triggers the servo motors",
    10000,
    NULL,
    1, // not time sensitive
    NULL,
    1
    );                             

  xTaskCreatePinnedToCore(
    ledCmdTask,
    "changes LED color",
    10000,
    NULL,
    1, // not time sensitive
    NULL,
    1
    );                             

  xTaskCreatePinnedToCore(
    settingCmdTask,
    "changes settings",
    10000,
    NULL,
    1, // not time sensitive
    NULL,
    1
    );

  xTaskCreatePinnedToCore(
    dynamicLightingChange,
    "updates color",
    10000,
    NULL,
    1, // not time sensitive
    &dynamicLEDTask,
    1
    );


  // suspend the dynamic lighting task until dynamic lighting is enabled by the user
  if (!dynamic_lighting) {
    vTaskSuspend(dynamicLEDTask);
  }
}


void setup() {
  Serial.begin(115200); 
  
  // create semaphores and mutex
  feed_semaphore = xSemaphoreCreateBinary();
  led_semaphore = xSemaphoreCreateBinary();
  setting_semaphore = xSemaphoreCreateBinary();
  payload_mutex = xSemaphoreCreateMutex();

  load_settings();

  // init ph sensor
  ph.init(PH_PIN, ESPADC, ESPVOLTAGE);
  ph.begin();


  // init temp sensor
  temperature.init(DS18S20_Pin);  

  // init ir sensor
  ir.init(IR_PIN, IR_THRESHOLD);

  // init servo
  si.init(SERVO_PIN);

  // init LEDs
  leds.init(ledPin, ledNum);

  // init LCD
  lcd.init(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
    
  // check if connected to computer
  userSetup();

  // init wifi and MQTT
  wiqtt.connectToWifi();
  wiqtt.setupMQTT();
  wiqtt.setCallback(callback); 

  // create tasks
  Serial.println("Creating Tasks");
  taskCreation();
}


void loop() {
  
}


void load_settings() {

  Preferences preferences;
  // recover saved wifi password
  preferences.begin("saved-values", false);
  String wifi_SSID = preferences.getString("wifi_SSID", "no value"); 
  String wifi_PWD = preferences.getString("wifi_PWD", "no value");
  wiqtt.setWifiCreds(wifi_SSID, wifi_PWD);

  // recover saved Alert username
  String alert_usr = preferences.getString("alert_usr", "no value");
  wiqtt.setAlertCreds(alert_usr);

  // recover saved timezone value
  gmtOffset_sec = preferences.getInt("time_zone", -5)*60*60;
  
  // recvoer saved setting
  publish_interval = preferences.getInt("publish_interval", 2);
  num_of_fish = preferences.getInt("num_of_fish", 3);
  dynamic_lighting = preferences.getBool("dynamic_lighting", false);
  send_alert = preferences.getBool("send_alert", false);
  
  preferences.end();
}


void userSetup() {
  Serial.setTimeout(500);
  Menu me;
  me.loop();
  //Serial.setTimeout(); // back to default value 
}


void dangerValueCheck(float tempVal, float pHVal, int foodLevel ) {

    String msg;

    // water tempurature value check
    if (tempVal >= MAX_TEMP) {
        msg = "High water temperature detected. Measured value: " + String(tempVal) + " deg-F";
        wiqtt.sendPushAlert(msg);
    }
    else if (tempVal <= MIN_TEMP) {
        msg = "Low water temperature detected. Measured value: " + String(tempVal) + " deg-F";
        wiqtt.sendPushAlert(msg);
    }
    
    // pH value check
    if (pHVal >= MAX_PH) {
        msg = "High water pH detected. Measured value: " + String(pHVal);
        wiqtt.sendPushAlert(msg);
    }
    else if (pHVal <= MIN_PH) {
        msg = "Low water pH detected. Measured value: " + String(pHVal);
        wiqtt.sendPushAlert(msg);
    }
    
    // food level check
    if (foodLevel == 0) {
        msg = "Low food level detected, refill food hopper";
        wiqtt.sendPushAlert(msg);
    }
    return;
}
