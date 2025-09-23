#include "MyConfig.h"
#include "WC_Task.h"


//SonarA21 sensor1;


void setup(){
  Serial.begin(115200);
  vTaskDelay(2000);
  Serial.println(F("Start controller ..."));
  
 // Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
 // Wire.setTimeOut(2000);

//  Wire.setClock(100000);
//  sensor1.init();

  tasksStart();

}




void loop(){
//   sensor1.init();
 //  float x = sensor1.getDistance();
 //  Serial.print("!!! dist=");
//   Serial.println(x);


   vTaskDelay(200000);
//   Serial.println("!!! Loop");
}
