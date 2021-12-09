#include "cam-stream.h"

CamStream cam;

void setup() {
  cam.configCamera("Fishwifi", "fishfood");
}
 void loop() {
   cam.http_resp();
   if(cam.getConnected() == true){
     cam.liveCam();
   }
 }
