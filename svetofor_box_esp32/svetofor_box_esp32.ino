#include "MyConfig.h"
#include "WC_Task.h"


//SonarA21 sensor1;


void setup(){
  Serial.begin(115200);
  vTaskDelay(2000);
  Serial.println(F("Start controller ..."));
/*  
  while(true){
      pinMode(PIN_OUT1,OUTPUT);
      pinMode(PIN_OUT2,OUTPUT);
      digitalWrite(PIN_OUT1,HIGH);
      delay(5000);
      digitalWrite(PIN_OUT2,HIGH);
      delay(5000);
      digitalWrite(PIN_OUT1,LOW);
      delay(5000);
      digitalWrite(PIN_OUT2,LOW);
      delay(5000);
  }
*/

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
