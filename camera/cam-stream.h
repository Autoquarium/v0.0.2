#include "esp_camera.h"
#include <WiFi.h>
#include <string> 

 
 
class CamStream {
    private:
        // AI Thinker / WRoom Pin Assignments
        const int PWDN_GPIO_NUM = 32;
        const int RESET_GPIO_NUM = -1;
        const int XCLK_GPIO_NUM = 0;
        const int SIOD_GPIO_NUM = 26;
        const int SIOC_GPIO_NUM = 27;
        
        const int Y9_GPIO_NUM = 35;
        const int Y8_GPIO_NUM = 34;
        const int Y7_GPIO_NUM = 39;
        const int Y6_GPIO_NUM = 36;
        const int Y5_GPIO_NUM = 21;
        const int Y4_GPIO_NUM = 19;
        const int Y3_GPIO_NUM = 18;
        const int Y2_GPIO_NUM = 5;
        const int VSYNC_GPIO_NUM = 25;
        const int HREF_GPIO_NUM = 23;
        const int PCLK_GPIO_NUM = 22;


        // Rachel's hotspot: "Verizon-SM-G930V-A5BE" "mtpg344#"
        char* ssid;
        char* password;

        WiFiServer server;
        WiFiClient live_client;
        bool connected = false;

        String index_html = "<meta charset=\"utf-8\"/>\n" \
                        "<style>\n" \
                        "#content {\n" \
                        "display: flex;\n" \
                        "flex-direction: column;\n" \
                        "justify-content: center;\n" \
                        "align-items: center;\n" \
                        "text-align: center;\n" \
                        "min-height: 100vh;}\n" \
                        "</style>\n" \
                        "<body bgcolor=\"#2986cc\"><div id=\"content\"><h2 style=\"color:#000000\">Autoquarium LIVE</h2><img src=\"video\"></div></body>";

        
        
    public:
        /**
         * @brief Begins serial with 115200 baud, configures ESP32Cam Wi-Fi and settings
         * 
         * @param ssid the name of the user's local Wi-Fi network
         * @param password the password of the user's local Wi-Fi network
         */
        void configCamera(const char* ssid = "Verizon-SM-G930V-A5BE", const char* password = "mtpg344#");


        /**
         * @brief Grabs a camera frame and sends to client
         * 
         */
        void liveCam();

        /**
         * @brief Constructs HTTP response to send to client
         * 
         */
        void http_resp();

        /**
         * @brief Returns connected bool (denotes that stream connection is active)
         * 
         */
        bool getConnected();
    
};
