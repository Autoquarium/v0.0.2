#include "cam-stream.h"

CamStream cam;

void setup() {
  cam.configCamera();
}
 void loop() {
   cam.http_resp();
   if(cam.getConnected() == true){
     cam.liveCam();
   }
 }
