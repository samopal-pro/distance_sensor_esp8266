#include "WC_Task.h"
char strID[16];
uint64_t chipID;


MySensor sensor;
float Distance = NAN, lastDistance = NAN;
bool SensorOn = false, lastSensorOn = false;  
ES_STAT stat1 = STAT_OFF, stat2 = STAT_OFF;
uint32_t msStat1  = 0, msStat2 = 0;
bool statRelay1 = false, statRelay2 = false;
bool inverseRelay1 = false, inverseRelay2 = false;
uint16_t eventRelay1 = 0, eventRelay2 = 0;
uint32_t msRelay1 = 0, msRelay2 = 0;
bool isChangeStat  = false; //Изменение отслеживания изменения состояния для немедленной отправки 
uint32_t ms0 = 0;
uint16_t bootCount;
bool isWiFiAlways1 = true; 
SemaphoreHandle_t sensorSemaphore;
/**
 * Старт всех параллельных задач
 */
void tasksStart() {

  configInit();
//  configDefault();
//  configSave();

  configRead();
  saveRead();
  bootCount = saveCount(); 
  chipID = ESP.getEfuseMac();
  sprintf(strID,"%012llX",chipID);
  Serial.printf("!!! ID %s ",strID);
  Serial.println(chipID,HEX);
  
  //    xTaskCreateUniversal(taskLed, "led", 2048, NULL, 2, NULL,CORE);
  sensorSemaphore = xSemaphoreCreateMutex();
  xTaskCreateUniversal(taskSensors, "sensors", 30000, NULL, 3, NULL, CORE);
  //    vTaskDelay(500);
  // xTaskCreateUniversal(taskPoll, "poll", 10240, NULL, 2, NULL, CORE);
 // vTaskDelay(500);
//  xTaskCreateUniversal(taskDisplay, "display", 20480, NULL, 2, NULL, CORE);
//  vTaskDelay(500);
 // xTaskCreateUniversal(taskModbus, "modbus", 10240, NULL, 5, NULL,CORE);
  vTaskDelay(1000);
  xTaskCreateUniversal(taskNet, "net", 10000, NULL, 3, NULL, CORE);
      vTaskDelay(1000);
  xTaskCreateUniversal(taskButton, "btn", 4096, NULL, 2, NULL,CORE);
}

/**
 * Задача работы с сенсорами
 * @param pvParameters
 */
void taskSensors(void *pvParameters) {
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! Sensors task start"));
#endif
   ledInit();
   sensor.init();
   pinMode(PIN_OUT1,OUTPUT);
   setRelay1(statRelay1); 
   pinMode(PIN_OUT2,OUTPUT);
   setRelay1(statRelay2); 
   if( bootCount < 1 ){
      ProcessingCalibrate(0);
   }
   else {
      Distance = jsonSave["DISTANCE"].as<float>();
      SensorOn = jsonSave["STATE_ON"].as<bool>();
   }
   while (true) {
      uint32_t ms = millis();

      xSemaphoreTake(sensorSemaphore, portMAX_DELAY);
      uint16_t _status = (uint16_t)sensor.get();
      Distance = sensor.Value->getLast();
      if( isnan(Distance) ){
         switch( jsonConfig["RGB1"]["NAN_MODE"].as<int>() ){
            case NAN_VALUE_IGNORE: 
               Serial.println("!!! NAN. Skiping");
               break;  
            case NAN_VALUE_BUSY:
               Serial.println("!!! NAN. BUSY");
               SensorOn = !SENSOR_GROUND_STATE;
               break;
            case NAN_VALUE_FREE:
               Serial.println("!!! NAN. FREE");
               SensorOn = SENSOR_GROUND_STATE;
               break;         

         }
      }
      else {
         if( ( jsonConfig["SENSOR"]["INSTALL"].as<int>() == INSTALL_TYPE_NORMAL &&
                abs(jsonConfig["SENSOR"]["DIST_GROUND"].as<float>() - Distance ) < jsonConfig["SENSOR"]["DIST_LIMIT"].as<float>() )|| 
             ( jsonConfig["SENSOR"]["INSTALL"].as<int>() == INSTALL_TYPE_OUTSIDE &&
                ( (Distance < jsonConfig["SENSOR"]["DIST_MIN1"].as<float>() )||(Distance > jsonConfig["SENSOR"]["DIST_MAX1"].as<float>() )))||
             ( jsonConfig["SENSOR"]["INSTALL"].as<int>() == INSTALL_TYPE_INSIDE &&
                ( (Distance < jsonConfig["SENSOR"]["DIST_MIN2"].as<float>() )||(Distance > jsonConfig["SENSOR"]["DIST_MAX2"].as<float>() )))){
              
            SensorOn = SENSOR_GROUND_STATE;
         }
         else {
            SensorOn = !SENSOR_GROUND_STATE;
         }
         checkChangeOn();
      }
      setLedDistance( Distance, SensorOn);
      processRelay1();
      processRelay2();
      if( ms0 == 0 || ms0 > ms || ms-ms0 > 5000 ){
         ms0 = millis();
         printStat("TM");
      } 

      xSemaphoreGive(sensorSemaphore);

      vTaskDelay((uint32_t)(jsonConfig["SENSOR"]["T_LOOP"].as<float>()*1000));
  }
}


void checkChangeOn(){
  uint32_t _ms = millis();
// Зафиксировано изменение состояния   
   if( SensorOn != lastSensorOn ){
// Если кнопка включена       
      if( SensorOn != SENSOR_GROUND_STATE ){
// Если не прошел таймаут от предыдущего состояния      
         if( stat1 == STAT_OFF ){
             stat1 = STAT_BT_ON;
             msStat1 = _ms;  
             printStat("Stat1 ON");
         }
         else {
             stat1 = STAT_OFF;
             printStat("Stat1 ON Reset");
         }
         if( stat2 == STAT_OFF ){
             stat2 = STAT_BT_ON;
             msStat2 = _ms;  
             printStat("Stat2 ON");
         }
         else {
             stat2 = STAT_OFF;
             printStat("Stat2 ON Reset");
         }
      }
      else {
         if( stat1 == STAT_OFF ){
             stat1 = STAT_BT_OFF;
             msStat1 = _ms;  
             printStat("Stat1 OFF");
         }
         else {
             stat1 = STAT_OFF;
             printStat("Stat1 OFF Reset");
         }
         if( stat2 == STAT_OFF ){
             stat2 = STAT_BT_OFF;
             msStat2 = _ms;  
             printStat("Stat2 OFF");
         }
         else {
             stat2 = STAT_OFF;
             printStat("Stat2 OFF Reset");
         }
      }
      lastSensorOn = SensorOn;
      saveSet(Distance,SensorOn);
   } 
//   WS_setDistance();
   uint32_t _tm = 0;
   if( stat1 == STAT_BT_ON  )  _tm = (uint32_t)(jsonConfig["RELAY1"]["DELAY_ON"].as<float>()*1000); 
   if( stat1 == STAT_BT_OFF )  _tm = (uint32_t)(jsonConfig["RELAY1"]["DELAY_OFF"].as<float>()*1000); 
   if( ( stat1 == STAT_BT_ON || stat1 == STAT_BT_OFF ) && (_tm == 0 || _ms < msStat1 || _ms - msStat1 >= _tm ) ) {
       printStat("Stat1 Fixed");
       stat1 = STAT_OFF;
       eventRelay1 = 1;
       isChangeStat = true;
   }
   _tm = 0;
   if( stat2 == STAT_BT_ON  )  _tm = (uint32_t)(jsonConfig["RELAY2"]["DELAY_ON"].as<float>()*1000); 
   if( stat2 == STAT_BT_OFF )  _tm = (uint32_t)(jsonConfig["RELAY2"]["DELAY_OFF"].as<float>()*1000); 
   if( ( stat2 == STAT_BT_ON || stat2 == STAT_BT_OFF ) && (_tm == 0 || _ms < msStat2 || _ms - msStat2 >= _tm ) ) {
       printStat("Stat2 Fixed");
       stat2 = STAT_OFF;
       eventRelay2 = 1;
       isChangeStat = true;
   }
   
}

void processRelay1(){
#if defined(PIN_OUT1)   
   if( PIN_OUT1 < 0 )return;
// Проверяем не менялся ли флаг инверсии
   if( jsonConfig["RELAY1"]["INVERSE"].as<bool>() != inverseRelay1 )setRelay1(statRelay1);   
   if( eventRelay1 == 0 )return;
   uint32_t ms = millis();
   if( SensorOn ){
      switch( jsonConfig["RELAY1"]["MODE"].as<int>() ){
         case RELAY_NORMAL : //Один раз включить и больше не трогать
            setRelay1(true);
            eventRelay1 = 0; 
            break;
         case RELAY_PULSE : //Если задан одиночный импульс
         case RELAY_PULSE2 : //Если задан одиночный импульс
            if( eventRelay1 == 1 ){ //Включить и задать время горения
               msRelay1 = ms + (uint32_t)(jsonConfig["RELAY1"]["T_PULSE"].as<float>()*1000);
               setRelay1(true);
               eventRelay1 = 2;   
            }
            else if( msRelay1 == 0 || ms > msRelay1 ){ //Если прошло время отключить и больше не трогать
               msRelay1 = 0;
               eventRelay1 = 0;
               setRelay1(false);              
            }
            break;
         case RELAY_PULSE_OFF  :   
             setRelay1(false);
             msRelay1 = 0;
             eventRelay1 = 0;
             break;
         case RELAY_PWM : //Если задан повторяющийся импульс
            if( eventRelay1 == 1 && (msRelay1 == 0 || ms > msRelay1) ){ //Включить и задать время горения
               msRelay1 = ms + (uint32_t)(jsonConfig["RELAY1"]["T_PULSE"].as<float>()*1000);
               setRelay1(true);
               eventRelay1 = 2;   
            }
            else if( eventRelay1 > 1 && (msRelay1 == 0 || ms > msRelay1) ){ //Выключить и задать время паузы
               msRelay1 = ms + (uint32_t)(jsonConfig["RELAY1"]["T_PAUSE"].as<float>()*1000);
               eventRelay1 = 1;
               setRelay1(false);              
            }
            break;
      }
   }
   else {
      switch( jsonConfig["RELAY1"]["MODE"].as<int>() ){  
         case RELAY_PULSE_OFF : //Если задан одиночный импульс
         case RELAY_PULSE2 : 
            if( eventRelay1 == 1 ){ //Включить и задать время горения
               msRelay1 = ms + (uint32_t)(jsonConfig["RELAY1"]["T_PULSE"].as<float>()*1000);
               setRelay1(true);
               eventRelay1 = 2;   
            }
            else if( msRelay1 == 0 || ms > msRelay1 ){ //Если прошло время отключить и больше не трогать
               msRelay1 = 0;
               eventRelay1 = 0;
               setRelay1(false);              
            }
            break;
         case RELAY_NORMAL : //Во всех режимах отключить и больше не трогать
         case RELAY_PULSE :
         case RELAY_PWM :
             setRelay1(false);
             msRelay1 = 0;
             eventRelay1 = 0;
             break;
      }
   }
#endif   
}

void processRelay2(){
#if defined(PIN_OUT2)   
   if( PIN_OUT2 < 0 )return;
// Проверяем не менялся ли флаг инверсии
   if( jsonConfig["RELAY2"]["INVERSE"].as<bool>() != inverseRelay2 )setRelay2(statRelay2);   
   if( eventRelay2 == 0 )return;
   uint32_t ms = millis();
   if( SensorOn ){
      switch( jsonConfig["RELAY2"]["MODE"].as<int>() ){
         case RELAY_NORMAL : //Один раз включить и больше не трогать
            setRelay2(true);
            eventRelay2 = 0; 
            break;
         case RELAY_PULSE : //Если задан одиночный импульс
         case RELAY_PULSE2 : //Если задан одиночный импульс
            if( eventRelay2 == 1 ){ //Включить и задать время горения
               msRelay2 = ms + (uint32_t)(jsonConfig["RELAY2"]["T_PULSE"].as<float>()*1000);
               setRelay2(true);
               eventRelay2 = 2;   
            }
            else if( msRelay2 == 0 || ms > msRelay2 ){ //Если прошло время отключить и больше не трогать
               msRelay2 = 0;
               eventRelay2 = 0;
               setRelay2(false);              
            }
            break;
         case RELAY_PULSE_OFF  :   
             setRelay2(false);
             msRelay2 = 0;
             eventRelay2 = 0;
             break;
         case RELAY_PWM : //Если задан повторяющийся импульс
            if( eventRelay2 == 1 && (msRelay2 == 0 || ms > msRelay2) ){ //Включить и задать время горения
               msRelay2 = ms + (uint32_t)(jsonConfig["RELAY2"]["T_PULSE"].as<float>()*1000);
               setRelay2(true);
               eventRelay2 = 2;   
            }
            else if( eventRelay2 > 1 && (msRelay2 == 0 || ms > msRelay2) ){ //Выключить и задать время паузы
               msRelay2 = ms + (uint32_t)(jsonConfig["RELAY2"]["T_PAUSE"].as<float>()*1000);
               eventRelay2 = 1;
               setRelay2(false);              
            }
            break;
      }
   }
   else {
      switch( jsonConfig["RELAY2"]["MODE"].as<int>() ){  
         case RELAY_PULSE_OFF : //Если задан одиночный импульс
         case RELAY_PULSE2 : 
            if( eventRelay2 == 1 ){ //Включить и задать время горения
               msRelay2 = ms + (uint32_t)(jsonConfig["RELAY2"]["T_PULSE"].as<float>()*1000);
               setRelay2(true);
               eventRelay2 = 2;   
            }
            else if( msRelay2 == 0 || ms > msRelay2 ){ //Если прошло время отключить и больше не трогать
               msRelay2 = 0;
               eventRelay2 = 0;
               setRelay2(false);              
            }
            break;
         case RELAY_NORMAL : //Во всех режимах отключить и больше не трогать
         case RELAY_PULSE :
         case RELAY_PWM :
             setRelay2(false);
             msRelay2 = 0;
             eventRelay2 = 0;
             break;
      }
   }
#endif   
}

void  setRelay1( bool stat){
   bool _stat;
   if( jsonConfig["RELAY1"]["INVERSE"].as<bool>() )_stat = !stat;
   else _stat = stat;
   Serial.printf("!!! Set Relay1 pin=%d stat=%d\n",(int)PIN_OUT1,(int)_stat);
   digitalWrite(PIN_OUT1, _stat);
// Сохраняем значения
   statRelay1    = stat;
   inverseRelay1 = jsonConfig["RELAY1"]["INVERSE"].as<bool>();
}

void  setRelay2( bool stat){
   bool _stat;
   if( jsonConfig["RELAY2"]["INVERSE"].as<bool>() )_stat = !stat;
   else _stat = stat;
   Serial.printf("!!! Set Relay2 pin=%d stat=%d\n",(int)PIN_OUT2,(int)_stat);
   digitalWrite(PIN_OUT2, _stat);
// Сохраняем значения
   statRelay2    = stat;
   inverseRelay2 = jsonConfig["RELAY2"]["INVERSE"].as<bool>();
}


void  setRelayPin(uint8_t pin, bool stat, bool is_inverse){
   bool _stat;
   if( is_inverse )_stat = !stat;
   else _stat = stat;
//   Serial.printf("!!! Set Relay pin=%d stat=%d\n",(int)pin,(int)_stat);
   digitalWrite(pin, _stat);
}




void printStat(char *msg){
   ms0 = millis();
#if defined(DEBUG_SERIAL)   
   Serial.print(F("!!! Sensor: "));
   Serial.print(Distance, 0);
   Serial.print(F(" Stat: "));
   Serial.print(SensorOn);
   Serial.print(F(" Event: "));
   Serial.println(msg);
#endif
}

/**
* Автокалибровка земли
*/
float CalibrateGround(){
   uint32_t ms_start = millis();
// Макимальное ограничение калиброка 30сек
   for( uint32_t ms=millis(); ms-ms_start<5000; ms=millis()){
// Делаем цикл измерения
      int n = 0;
      float distArray[jsonConfig["CALIBR"]["NUMBER"].as<int>()];
      float distAvg = 0, distDiv = 0;
      for( int i=0; i<jsonConfig["CALIBR"]["NUMBER"].as<int>(); i++){
         uint16_t _status = (uint16_t)sensor.get();
         float L = sensor.Value->getLast();
         if( !isnan(L) ){
            distAvg += L;
            distArray[n++] = L;
//            PrintValue();
         }
         delay(100);
      }
      if( n < 2 )continue;
      distAvg /= n;
      for( int i=0; i<n; i++)distDiv += (distArray[i]-distAvg)*(distArray[i]-distAvg);
      distDiv = sqrt(distDiv/n);
      Serial.print("!!! Avg=");
      Serial.print(distAvg);
      Serial.print(",Div=");
      Serial.print(distDiv);
      Serial.print(",n=");
      Serial.println(n);
      if( distDiv/distAvg < RELIABILITY_PROC )return distAvg;
      Serial.println("??? Calibrate fail. Skipping ...");
   }
   return NAN;
}


bool ProcessingCalibrate(uint32_t _tm){
   ledSetColor(COLOR_GROUND,false);
//   ledSetBaseMode(LED_BASE_NONE,true);
   if(_tm>0)delay(_tm);
//   ledSetBaseMode(LED_BASE_GROUND);
   float x = CalibrateGround();
   if( isnan(x) ){
      ledSetColor(COLOR_ERROR,false);
      vTaskDelay(2000);
//      ledRestoreMode();
      Serial.println("??? Calibrate ground fail");
      ledRestoreColor();
      return false;
   }
   else {
//      ledSetBaseMode(LED_BASE_SAVE,true);
      ledRestoreColor();
      vTaskDelay(1000);
//      ledRestoreMode();
      Serial.printf("!!! Calibrate ground value %d\n",(int)x);
      jsonConfig["SENSOR"]["DIST_GROUND"]  = (int)x;
      jsonConfig["SENSOR"]["DIST_MIN1"]    = (int)x;
      jsonConfig["SENSOR"]["DIST_MAX1"]    = (int)x;
      jsonConfig["SENSOR"]["DIST_MIN2"]    = (int)x;
      jsonConfig["SENSOR"]["DIST_MAX2"]    = (int)x;
      configSave(); 
      configRead();      
      return true;
   }
}

void taskButton(void *pvParameters){
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! Button task start"));
#endif
   pinMode(PIN_BTN,INPUT);
   SButton btn(PIN_BTN);
   btn.SetLongClick(10000);
   bool is_btn_click = false;
   uint16_t btn_count = 0;
   uint32_t ms_btn = 0;
   bool white_flag = false;
   while(true){
       uint32_t cur_ms = millis();
       if( ms_btn == 0 || ms_btn > cur_ms || cur_ms - ms_btn > 4000 ){ btn_count = 0; }
       switch(btn.Loop()){
          case SB_NONE:
              is_btn_click = false;             
              break;     
          case SB_WAIT:
              if( is_btn_click == false ){
                  ledSetColor(COLOR_NONE,true);
                  white_flag   = false;
              }
              is_btn_click = true;            
              if( btn.Time < 2000 )
                  if( white_flag)ledSetColor(COLOR_SAVE);
                  else ledSetColor(COLOR_NONE);
              else ledSetColor(COLOR_GROUND);             
              white_flag = !white_flag;
          case SB_CLICK:
              Serial.printf("!!! BTN click %d\n",(int)btn_count);
              btn_count++;
              if( btn_count == 5 ){
                btn_count = 0;
                jsonConfig["SYSTEM"]["NAME"]        = DEVICE_NAME;
                ledSetColor(COLOR_ERROR);
                configSave();     
                delay(1000);    
                ESP.restart();  
              }
              ms_btn = cur_ms;
              Serial.printf("!!! BTN click %d\n",btn.Time);
              if( btn.Time > 2000 ){
//                 ledSetExtMode(LED_EXT_BTN3);
//                 if( EA_Config.isWiFiAlways == false  && isWiFiAlways1 == false )
//                    if( w_stat2 == EWS_AP_MODE )WiFi_stop("Stop AP User");
//                    else WiFi_startAP();
                 xSemaphoreTake(sensorSemaphore, portMAX_DELAY);
                 Serial.printf("!!! Calibrate sonar %d\n",(int)Distance);
                 ProcessingCalibrate(jsonConfig["CALIBR"]["DELAY_START"].as<uint32_t>()*1000);

//                 EA_Config.GroundLevel = Distance;
                 xSemaphoreGive(sensorSemaphore);

//             EA_default_config();
//                EA_read_config();
 
              }
              break;
          case SB_LONG_CLICK:
              Serial.println("!!! BTN long click");   
              jsonSave["BOOT_COUNT"]  = 0;
              saveSave();
              jsonConfig["SYSTEM"]["PASS0"]       = DEVICE_PASS0;               //Пароль суперадминистратора
              jsonConfig["SYSTEM"]["PASS1"]       = DEVICE_PASS1;               //Пароль администратора
              configSave();
              delay(1000);    
              ESP.restart();  
             break;
       }
       
   }

}

/**
 * Задача передача данных через WiFi
 * @param pvParameters
 */
void taskNet( void *pvParameters ){
#if defined(DEBUG_SERIAL)
    Serial.println(F("!!! WiFi task start"));
#endif
   if( jsonConfig["SYSTEM"]["AP_START"].as<bool>() ||  isWiFiAlways1 )WiFi_startAP();
//   HTTP_begin();
   while(true){
      HTTP_loop();
      vTaskDelay(50);      
   }


}
