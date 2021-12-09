#include "menu.h"

int Menu::send_start() {
    unsigned long start = millis();
    while (millis() - start < 3000) {
        Serial.println("blub-blub-blub");
        if (Serial.available()) {
            String x = Serial.readString();
            if (x == "hello-fish") {
                return 1;
            }
        }
    }
    return 0;
}


String Menu::getInput() {
    String ret;
    Serial.println(get_input);
    while(!Serial.available());
    ret += Serial.readString();// read the incoming data as string
    return ret;
}


String Menu::getPassword() {
    String ret;
    Serial.println(get_pass);
    while(!Serial.available());
    ret += Serial.readString();// read the incoming data as string
    return ret;
}


int Menu::makeSelection() {
    return getInput().toInt();
}


void Menu::adminFunction() {
    // TODO:
    // 0) Back // back to main menu
    // 1) Print all
    // 2) Edit
        // enter var_name
        // enter new_value
}


void Menu::testWifi(String ssid_in, String passwrd_in) {

    char ssid[30];
    char passwrd[30];

    ssid_in.toCharArray(ssid, ssid_in.length() + 1);
    passwrd_in.toCharArray(passwrd, passwrd_in.length() + 1);


    int status = WL_IDLE_STATUS;
    Serial.println("Testing Connection . . .");
    
    WiFi.begin(ssid, passwrd);
    
    unsigned long start = millis();
    while (status != WL_CONNECTED) {
        status = WiFi.begin(ssid, passwrd);
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
        if (millis() - start > 40000) {
            Serial.println("Connection Failed");
            return;
        }
    }

    Serial.println("Connection Successful!");
    Serial.println("Connection Strength: " + String(WiFi.RSSI()));
}


int Menu::adminMenu() {
    Serial.println(clear_screen);
    Serial.println("Admin Menu\n");
    
    selection = makeSelection(); // TODO: replace with adminFunction();
    current_menu = &Menu::menu1;
    selection = -1;
    return 6;
}


int Menu::adminLogin() {
    Serial.println(clear_screen);
    Serial.println("Admin Login\n");
    Serial.println("Enter Admin Password: ");
    String password = getPassword();

    if (password != "test") {
        Serial.println("\n\nPassword Incorrect!");
        Serial.println("---------------");
        Serial.println("  0) Back"); // go to main menu
        Serial.println("  1) Retry"); // just clear and keep going
        selection = makeSelection();
    }
    else {
        current_menu = &Menu::adminMenu;
        selection = -1;
    }
    return 5;
}


int Menu::deviceInfoMenu() {
    Serial.println(clear_screen);
    Serial.println("Device Information:");
    Serial.println("    Serial Number: 1");
    Serial.println("    Dashboard URL: https://autoquarium.app/dashboard/d/8ecf97a7-5e0c-41dc-bd1a-770f5e9f716a");
    Serial.println("");

    // get values for printing
    preferences.begin("saved-values", false);
    String ssid = preferences.getString("wifi_SSID", "no value"); 
    String password = preferences.getString("wifi_PWD", "no value");
    String alert_usr = preferences.getString("alert_usr", "no value");
    int time_off = preferences.getInt("time_zone", -5);
    preferences.end();

    Serial.println("Other Information:");
    Serial.println("    WiFi SSD: " + ssid);
    Serial.println("    WiFi Password: " + password);
    Serial.println("    PushOver User Key: " + alert_usr);
    Serial.println("    Timezone offset: " + String(time_off));
    Serial.println("");
    Serial.println("----------------------");
    Serial.println("  0) Back");
    selection = makeSelection();
    return 4;
}


int Menu::timeZoneSetup() {
    Serial.println(clear_screen);
    Serial.println("Setup TimeZone\n");
    Serial.println("input number and press enter:");
    Serial.println("  0) EST (GMT-5:00)");
    Serial.println("  1) CST (GMT-6:00)");
    Serial.println("  2) MST (GMT-7:00)");
    Serial.println("  3) PST (GMT-8:00)");
    int timezone = (makeSelection() + 5) * -1;

    // save timezone to non-vol memory
    preferences.begin("saved-values", false);
    preferences.putInt("time_zone", timezone); 
    preferences.end();
    return 3;
}


int Menu::notiSetup() {
    Serial.println(clear_screen);
    Serial.println("Enter PushOver User Key: ");
    String alert_usr = getInput();
    
    preferences.begin("saved-values", false);
    preferences.putString("alert_usr", alert_usr); 
    preferences.end();
    return 2;
}


int Menu::wifiMenu() {
    Serial.println(clear_screen);
    Serial.println("Enter WiFi SSID: ");
    String wifi_SSID = getInput();
    Serial.println("Enter WiFi Pasword: ");
    String wifi_PWD = getInput();
    testWifi(wifi_SSID, wifi_PWD);
    delay(1000);

    Serial.println("\n\n---------------");
    Serial.println("  0) Back");
    Serial.println("  1) Re-enter"); // reload wifiMenu
    Serial.println("  2) Save"); // save the vars locally
    selection = makeSelection();

    if (selection == 2) {
        preferences.begin("saved-values", false);
        preferences.putString("wifi_SSID", wifi_SSID); 
        preferences.putString("wifi_PWD", wifi_PWD);
        preferences.end();
    }

    return 1;
}


int Menu::menu1() {
    Serial.println(clear_screen);
    Serial.println("input number and press enter:");
    Serial.println("  0) Quit");
    Serial.println("  1) WiFi setup");
    Serial.println("  2) Notifcation setup");
    Serial.println("  3) Timezone setup");
    Serial.println("  4) Device info");
    Serial.println("  5) Admin tools");
    Serial.println("");
    selection = makeSelection();
    return 0;
}


void Menu::execute(int menu_val) {
    
    if (selection == -1) {
    return;
    }
    
    // main menu
    if (menu_val == 0) {
        if (selection == 0) {
            Serial.println(clear_screen);
            Serial.println("Goodbye!");
            Serial.println(exit_cli);
            ESP.restart(); // start the entire processor
        } else if (selection == 1) {
            current_menu = &Menu::wifiMenu;
        } else if (selection == 2) {
            current_menu = &Menu::notiSetup;
        } else if (selection == 3) {
            current_menu = &Menu::timeZoneSetup;
        } else if (selection  == 4) {
            current_menu = &Menu::deviceInfoMenu;
        } else if (selection  == 5) {
            current_menu = &Menu::adminLogin;
        }
    } else if (menu_val == 1) {
        if (selection == 0 || selection == 2) {
            current_menu = &Menu::menu1;
        }
    } else if (menu_val == 2) {
        current_menu = &Menu::menu1;
    } else if(menu_val == 3) {
        current_menu = &Menu::menu1;
    } else if(menu_val == 4) {
        current_menu = &Menu::menu1;
    }
}


void Menu::loop() {
    if (send_start()) {
        current_menu = &Menu::menu1;
        while(1) {
            int menu_val = (this->*current_menu)(); //Calling a member function from another member function using pointer to member
            execute(menu_val);
        }
    }
}
