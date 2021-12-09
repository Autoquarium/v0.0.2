#include <Arduino.h>
#include <Preferences.h>
#include <time.h>

#include "menu.h"              /* cli and firsttime setup */
#include "dfrobot-esp-ph.h"    /* pH sensor lib */
#include "ir-sensor.h"         /* low food reading */
#include "fish-servo.h"        /* moving servo */
#include "led-array.h"         /* led lighting control */ 
#include "lcd.h"               /* LCD screen control */
#include "fish-mqtt.h"         /* WiFi, MQTT, and Notifications */
#include "temp-sensor.h"       /* water temp sensor lib */


// FOR TESTING: change device ID if you're working on the non-production device
String device_id = "autoq-prod";; 


// semaphores for subscribed MQTT cmds - USE BINARY SEMAPHORES BECAUSE WE ARE TRIGGERING ANOTHER TASK TO RUN
SemaphoreHandle_t feed_semaphore;
SemaphoreHandle_t led_semaphore;
SemaphoreHandle_t autoled_semaphore;
SemaphoreHandle_t autofeed_semaphore;
SemaphoreHandle_t mqtt_semaphore;


// mutex for CMD_PAYLOAD - USE MUTEX BECAUSE ITS A SHARED RESOURCE:
// https://stackoverflow.com/questions/62814/difference-between-binary-semaphore-and-mutex
SemaphoreHandle_t payload_mutex;

// setting variables - loaded from memory / set in CLI or web app
long gmtOffset_sec;
int num_of_fish;
bool dynamic_lighting;
bool auto_feed;
bool send_alert = false;

// Danger cutoof values
const int MAX_TEMP  = 90;
const int MIN_TEMP = 70;
const int MAX_PH = 9;
const int MIN_PH = 5;


// pH sensor
const float ESPADC = 4096.0;   //the esp Analog Digital Convertion value
const int ESPVOLTAGE = 3300; //the esp voltage supply value
const int PH_PIN = 33;    //pH sensor gpio pin
DFRobotESPpH ph;

// LCD pins

// old pins
/*
const int TFT_DC = 17;
const int TFT_CS = 15;
const int TFT_RST = 5;
const int TFT_MISO = 19;         
const int TFT_MOSI = 23;           
const int TFT_CLK = 18;
*/

const int TFT_CS = 22;
const int TFT_RST = 5;
const int TFT_DC = 21;
const int TFT_MOSI = 23;           
const int TFT_CLK = 18;
const int TFT_MISO = 19; 

LCD lcd;
TempSensor temperature;

// ir sensor
const int IR_PIN = 32;
const int IR_THRESHOLD = 20; //TODO change to reflect values in enclosure
//int food_level;
IRSensor ir;

//Temperature chip
int DS18S20_Pin = 13; //DS18S20 Signal pin on digital 2

// servo/feed
const int SERVO_PIN = 26;
const int DELAY_BETWEEN_ROTATION = 1000;
const int MIN_FEED_INTERVAL = 1; //1 minute
FishServo si;
int previous_feed_time;

// LED array
const int ledPin = 2;
const int ledNum = 50;
bool leds_off = false;

LEDArray leds;

// MQTT, WiFi and notification
FishMqtt wiqtt;
int publish_interval; // in minutes
char CMD_PAYLOAD[30];
int wifiLedPin = 12;

//function prototypes
void userSetup();
void load_settings();
void dangerValueCheck( float tempVal, float pHVal );


// task handlers
TaskHandle_t dynamicLEDTask;
TaskHandle_t autoFeedTask;



//******************
// HELPER FUNCTIONS
//******************

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

  // recover saved time values
  previous_feed_time = preferences.getInt("previous_feed_time", -1);
  gmtOffset_sec = preferences.getInt("time_zone", -5)*60*60;
  
  // recover saved setting
  publish_interval = preferences.getInt("publish_interval", 5);
  num_of_fish = preferences.getInt("num_of_fish", 1);
  dynamic_lighting = preferences.getBool("dynamic_lighting", false);
  auto_feed = preferences.getBool("auto_feed", false);
  
  // enable notifications only if alert credential value has been configured
  if (alert_usr != "no value") {
    send_alert = true;
  }
  preferences.end();
}


void userSetup() {
  Serial.setTimeout(500);
  Menu me(wifiLedPin);
  me.loop();
}


void dangerValueCheck(float tempVal, float pHVal) {

    String s1;
    String s2;
    
    // water tempurature value check
    if (tempVal >= MAX_TEMP && tempVal < 150) {
        s1 = "High water temperature detected. Measured value: " + String(tempVal) + " deg-F";
    }
    else if (tempVal <= MIN_TEMP && tempVal > 0) {
        s1 = "Low water temperature detected. Measured value: " + String(tempVal) + " deg-F";
    }
    
    // pH value check
    if (pHVal >= MAX_PH) {
        s2 = "High water pH detected. Measured value: " + String(pHVal);
    }
    else if (pHVal <= MIN_PH) {
        s2 = "Low water pH detected. Measured value: " + String(pHVal);
    }

    // needed after push notifications
    if (s1.length() != 0) {
      wiqtt.sendPushAlert(s1);
    }
    if (s2.length() != 0) {
      wiqtt.sendPushAlert(s2);
    }

    wiqtt.MQTTreconnect();
    return;
}

/**
 * @brief Get the current time hr:min
 * 
 * @return current time in format HHMM
 */
int getTime(){

  String current_min;
  String current_hour;
  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return -1;
  }
  
  current_hour = String(timeinfo.tm_hour);
  if (timeinfo.tm_min < 10) {
    current_min = String("0") + String(timeinfo.tm_min);
  } else {
    current_min = String(timeinfo.tm_min);
  }
  current_hour = current_hour + current_min;
  return current_hour.toInt();
}

/*
 * @brief returns time1-time2 in hour,min format
 * 
 * @params time1, time2
 */
int getTimeDiff(int time1, int time2){
  int diff = time1-time2;
  int time2_adj;
  if(diff < 0){
    time2_adj = 2400 - time2;
    diff = time1 + time2_adj;
  }
  return diff;
}


// ************
// MAIN TASKS
// ************

// renamed from checkIncomingCmds to keepAliveMQTT
void keepAliveMQTT( void * parameter ){
  // keep track of last wake
  portTickType xLastWakeTime;

  // set delay period (250 ms)
  portTickType xPeriod = ( 250 / portTICK_RATE_MS );

  for(;;){
    if (wiqtt.checkWificonnection()) {
      xSemaphoreTake(mqtt_semaphore, portMAX_DELAY);
      wiqtt.loop();
      xSemaphoreGive(mqtt_semaphore);
    } else {
      wiqtt.connectToWifi();
    }
    vTaskDelay(xPeriod); // run every 250 ms
  } 
}


void publishSensorVals( void * parameter ) {
  portTickType xLastWakeTime;

  // convert publish interval from minutes into ms
  int delay_in_ms = publish_interval * 60 * 1000;
  portTickType xPeriod = ( delay_in_ms / portTICK_RATE_MS );
  xLastWakeTime = xTaskGetTickCount ();
  
  for( ;; ) {
    Serial.println("Publishing new sensor values to broker");

    // get water temperature
    float temp_read = temperature.getTemp();
    Serial.print("Temp sensor: ");
    Serial.println(temp_read);

    // get water pH
    float pH_read = ph.getPH(25);//(temp_read-32)/1.8); //convert temperature to celcius TODO: replace for final submit
    Serial.print("pH sensor: ");
    Serial.println(pH_read);

    // publish data
    xSemaphoreTake(mqtt_semaphore, portMAX_DELAY);
    wiqtt.publishSensorVals(temp_read, pH_read, getTime());
    if (send_alert) dangerValueCheck(temp_read, pH_read);
    xSemaphoreGive(mqtt_semaphore);
    
    // Wait for the next cycle.
    vTaskDelayUntil( &xLastWakeTime, xPeriod );
  }
}


void dynamicLightingChange( void * parameter ) {
  portTickType xLastWakeTime;
  
  int delay_in_ms = 1* 60 * 1000; // 1 minute to ms
  portTickType xPeriod = ( delay_in_ms / portTICK_RATE_MS );
  xLastWakeTime = xTaskGetTickCount();

  for( ;; ) {
    vTaskDelay(xPeriod );
    if (!leds_off) {
      Serial.println("Dynamic lighting change");
      leds.updateDynamicColor(getTime());
    }
  }
}


/*
 * @brief Autofeed fish task. Will be suspended until auto_feed is enabled by the user.
 *        Checks every 10 minutes whether to feed the fish (12 hour feed interval).
 * 
 * @params time1, time2
 */
void autoFeedChange( void *pvParameters ) {
  portTickType xLastWakeTime;
  // convert publish interval from minutes into ms
  int delay_in_ms = 10 * 60 * 1000; //10 minutes to ms
  portTickType xPeriod = ( delay_in_ms / portTICK_RATE_MS );
  
  for ( ;; ) {
    Serial.println("Auto feed check");
    if((previous_feed_time == -1) || (getTimeDiff(getTime(), previous_feed_time) >= MIN_FEED_INTERVAL)){
      for(int i = 0; i < num_of_fish; i++) {
        si.fullRotation(1000);
      }

      bool food_level = ir.getFoodLevel() == 1;
      xSemaphoreTake(mqtt_semaphore, portMAX_DELAY);
      wiqtt.publishFoodLevel(food_level);
      xSemaphoreGive(mqtt_semaphore);
      
      previous_feed_time = getTime();
    }
    vTaskDelay( xPeriod );
  }
}


// *************************
// CMD TASKS
// (triggered in callback)
// **************************

void feedCmdTask( void *pvParameters){
  for ( ;; ) {
    xSemaphoreTake(feed_semaphore, portMAX_DELAY);
    Serial.println("Feed button pressed");
    if((previous_feed_time == -1) || (getTimeDiff(getTime(), previous_feed_time) >= MIN_FEED_INTERVAL)){
      
      int temp_num = num_of_fish;
      num_of_fish = atoi(CMD_PAYLOAD);
      Serial.print(num_of_fish);
      
      for(int i = 0; i < num_of_fish; i++) {
        si.fullRotation(1000);
      }
      
      // update dashboard food levels
      bool food_level = ir.getFoodLevel();

      xSemaphoreTake(mqtt_semaphore, portMAX_DELAY);
      wiqtt.publishFoodLevel(food_level == 1);
      xSemaphoreGive(mqtt_semaphore);
      
      previous_feed_time = getTime();
      
    }
    else{
      Serial.println("Unable to feed, time interval too close.");
      xSemaphoreTake(mqtt_semaphore, portMAX_DELAY);
      wiqtt.sendPushAlert("Unable to feed, you already fed your fish today!");
      wiqtt.MQTTreconnect();
      xSemaphoreGive(mqtt_semaphore);
    }
  }

}

void ledCmdTask( void *pvParameters ) {
  for ( ;; ) {
    xSemaphoreTake(led_semaphore, portMAX_DELAY);
    Serial.println("change led color");  
    
    //transition to new color
    int r = atoi(strtok(CMD_PAYLOAD, ","));
    int g = atoi(strtok(NULL, ","));
    int b = atoi(strtok(NULL, ","));
    
    if (r = -1) {
      leds.updateDynamicColor(getTime());
    } else {
      leds.changeColor(r, g, b);
    }

    if (!r && !g && !b) {
      leds_off = true;
    } else {
      leds_off = false;
    }
  }
}


// *************************
// AUTO TASKS
// (run automatically)
// **************************
void settingCmdTaskAutofeed( void *pvParameters ) {
  for ( ;; ) {
    xSemaphoreTake(autofeed_semaphore, portMAX_DELAY);
    
    if (!strcmp(CMD_PAYLOAD, "false")) {
      Serial.println("autofeed disabled");
      vTaskSuspend( autoFeedTask );
      auto_feed = false;
    } else {
      Serial.println("autofeed enabled");
      vTaskResume( autoFeedTask );
      auto_feed = true;
    }
    
    // save updated values to non-volitile memory
    Preferences preferences;
    preferences.begin("saved-values", false);
    preferences.putBool("auto_feed", auto_feed);
    preferences.end();
  }
}


void settingCmdTaskAutoled( void *pvParameters ) {
  for ( ;; ) {
    xSemaphoreTake(autoled_semaphore, portMAX_DELAY);
    
    if (!strcmp(CMD_PAYLOAD, "false")) {
      Serial.println("dynamic LED disabled");
      vTaskSuspend( dynamicLEDTask );
      dynamic_lighting = false;
    } else {
      Serial.println("dynamic LED enabled");
      vTaskResume( dynamicLEDTask );
      dynamic_lighting = true;
      leds.updateDynamicColor(getTime());
    }
    
    // save updated values to non-volitile memory
    Preferences preferences;
    preferences.begin("saved-values", false);
    preferences.putBool("dynamic_lighting", dynamic_lighting);
    preferences.end();
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

    Serial.println("in callback");
  
    // save payload to CMD_PAYLOAD
    // need to use a mutex for the payload so that the cmd tasks are not reading while this is writting
    int i = 0;
    //xSemaphoreTake(payload_mutex, portMAX_DELAY);
    for (; i < length; i++) {
      CMD_PAYLOAD[i] = (char) payload[i];
    }
    CMD_PAYLOAD[i] = '\0';
    //xSemaphoreGive(payload_mutex);
    
    // FEEDING CMDS
    String topic_str = String(topic);
    if (topic_str == device_id + "/cmds/feed") {
      xSemaphoreGive(feed_semaphore);
    }

    // LIGHTING CMDS
    else if (topic_str == device_id + "/cmds/leds") {
      xSemaphoreGive(led_semaphore);
    }

    // SETTING CHANGES
    // dynamic lighting
    else if (topic_str == device_id + "/cmds/settings/autoled") {
      xSemaphoreGive(autoled_semaphore); 
    }
  
    // autofeed
    else if (topic_str == device_id + "/cmds/settings/autofeed") {
      xSemaphoreGive(autofeed_semaphore); 
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
    3, // this task is NOT vital for correct system operation
    NULL,
    1
    );                             

  xTaskCreatePinnedToCore(
    keepAliveMQTT,
    "check Incoming MQTT msgs",
    10000,
    NULL,
    4, // this task is vital for correct system operation
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
    settingCmdTaskAutofeed,
    "changes autofeed settings",
    10000,
    NULL,
    1, // not time sensitive
    NULL,
    1
    );
  
  xTaskCreatePinnedToCore(
    settingCmdTaskAutoled,
    "changes dynamic ligting settings",
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

  xTaskCreatePinnedToCore(
    autoFeedChange,
    "auto feeds fish",
    10000,
    NULL,
    1, // not time sensitive
    &autoFeedTask,
    1
    );

  // suspend the dynamic lighting task until dynamic lighting is enabled by the user
  //if (!dynamic_lighting) {
    vTaskSuspend(dynamicLEDTask);
  //}

  // suspend the auto feed task until auto feed is enabled by the user
  //if (!auto_feed) {
   vTaskSuspend(autoFeedTask);
  //}
}


void setup() {
  Serial.begin(115200); 

  // create semaphores and mutex
  feed_semaphore = xSemaphoreCreateBinary();
  led_semaphore = xSemaphoreCreateBinary();
  autoled_semaphore = xSemaphoreCreateBinary();
  autofeed_semaphore = xSemaphoreCreateBinary();
  mqtt_semaphore = xSemaphoreCreateBinary();
  payload_mutex = xSemaphoreCreateMutex();


  xSemaphoreGive(mqtt_semaphore);

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

  
  // turn on front light
  wiqtt.ledSetup(wifiLedPin);

  // check if connected to computer
  userSetup();

  // init LCD
  lcd.init(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

  // init LEDs - needs to come after userSetup() to prevent power surge on USB
  leds.init(ledPin, ledNum);

  // init wifi and MQTT
  wiqtt.setDeviceId(device_id);
  //wiqtt.connectToWifi();
  //wiqtt.setupMQTT();
  //wiqtt.setCallback(callback);

  // Setup clock
  configTime(gmtOffset_sec, 0, "pool.ntp.org");

  // create tasks
  Serial.println("Creating Tasks");
  taskCreation();
}

void loop() { 
    ph.calibration();
   //ph.getPH(25);
    //delay(1000);
}
