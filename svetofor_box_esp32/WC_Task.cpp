#include "WC_Task.h"
char strID[16];
uint64_t chipID;


MySensor *sensor;
TEvent *EventSensor, *EventRelay1, *EventRelay2;
TEvent *EventBusy1, *EventBusy2;
TEventRGB *EventRGB1, *EventRGB2;
//TEventRGB  *SaveEventRGB2, *SaveEventCalibrate1, *SaveEventCalibrate2;
TSaveRGB *SaveRGB1, *SaveRGB2;


TEventMP3 *EventMP3;
TEvent *EventCalibrate; 

bool isDFPlayer = false;
bool isPlayMP3  = false;
CMD_MP3_t cmdMP3 = CMP3_NONE;
int arg1MP3 = 0, arg2MP3 = 0;


float Distance = NAN, lastDistance = NAN;
SENSOR_STAT_t SensorOn = SS_NONE, lastSensorOn = SS_NONE;  
ES_STAT stat1 = STAT_OFF, stat2 = STAT_OFF;
uint32_t msStat1  = 0, msStat2 = 0;
bool statRelay1 = false, statRelay2 = false;
bool inverseRelay1 = false, inverseRelay2 = false;
uint16_t eventRelay1 = 0, eventRelay2 = 0;
uint32_t msRelay1 = 0, msRelay2 = 0;

CALIBRATION_MODE_t calibrMode = CM_NONE;
float calibrAvg = 0;
uint16_t calibrCount = 0;

uint32_t ms0 = 0, ms1 = 0;

bool isChangeNan   = true;
bool isChangeStat  = false; //Изменение отслеживания изменения состояния для немедленной отправки 
uint16_t bootCount;
bool isWiFiAlways1 = true; 
SemaphoreHandle_t sensorSemaphore;
uint32_t msSendHttp = 0;

HTTPClient httpClient;

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
  xTaskCreateUniversal(taskMP3, "mp3", 10000, NULL, 1, NULL, CORE);
  xTaskCreateUniversal(taskEvents, "events", 30000, NULL, 3, NULL, CORE);

  xTaskCreateUniversal(taskSensors, "sensors", 30000, NULL, 4, NULL, CORE);
  //    vTaskDelay(500);
  // xTaskCreateUniversal(taskPoll, "poll", 10240, NULL, 2, NULL, CORE);
 // vTaskDelay(500);
//  xTaskCreateUniversal(taskDisplay, "display", 20480, NULL, 2, NULL, CORE);
//  vTaskDelay(500);
 // xTaskCreateUniversal(taskModbus, "modbus", 10240, NULL, 5, NULL,CORE);
  vTaskDelay(500);
  xTaskCreateUniversal(taskNet, "net", 10000, NULL, 3, NULL, CORE);
  vTaskDelay(500);
  xTaskCreateUniversal(taskButton, "btn", 4096, NULL, 4, NULL,CORE);
}

/**
 * Задача работы с событиями
 * @param pvParameters
 */
void taskEvents(void *pvParameters) {
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! Events task start"));
#endif
//jsonConfig["RGB2"]["IS_MP3"] = false;
   EventSensor         = new TEvent(0,0,handleSensor);
   EventRelay1         = new TEvent((uint32_t)(jsonConfig["RELAY1"]["DELAY_ON"].as<float>()*1000),(uint32_t)(jsonConfig["RELAY1"]["DELAY_OFF"].as<float>()*1000),handleRelay1);
   EventRelay2         = new TEvent((uint32_t)(jsonConfig["RELAY2"]["DELAY_ON"].as<float>()*1000),(uint32_t)(jsonConfig["RELAY2"]["DELAY_OFF"].as<float>()*1000),handleRelay2);
   EventRGB1           = new TEventRGB(0,0,handleRGB1);
   EventRGB2           = new TEventRGB(0,0,handleRGB2);
   SaveRGB1            = new TSaveRGB(EventRGB1,1);
   SaveRGB2            = new TSaveRGB(EventRGB2,2);
   EventBusy1          = new TEvent(jsonConfig["MP3"]["BUSY1"]["DELAY"].as<uint32_t>(),0,handleBusy1);
   EventBusy2          = new TEvent(jsonConfig["MP3"]["BUSY2"]["DELAY"].as<uint32_t>(),0,handleBusy2);
//   SaveEventRGB2       = new TEventRGB(0,0,handleRGB2);
//   SaveEventCalibrate1 = new TEventRGB(0,0,handleRGB1);  
//   SaveEventCalibrate2 = new TEventRGB(0,0,handleRGB2);  
//   EventMP3            = new TEventMP3(0,0,handleMP3,1,1,false);
   EventCalibrate      = new TEvent(0,0,handleCalibrate);

   setEventRGB1( ET_NORMAL, 0, 0, COLOR_NAN, COLOR_NONE); 
   setEventRGB2( ET_NORMAL, 0, 0, COLOR_NAN, COLOR_NONE); 

//   Serial.printf("!!! EventRelay1 Init %d %d \n",(int)EventRelay1->Type,(int)EventRelay1->State);
   while (true) {
      uint32_t ms = millis();
      EventSensor->loop();
//      if( EventSensor->changeState()  )Serial.printf("!!! EventSensor Loop %d %d \n",(int)EventSensor->Type,(int)EventSensor->State);
      EventRelay1->loop();
      if( EventRelay1->changeState()  )Serial.printf("!!! EventRelay1 Loop %d %d \n",(int)EventRelay1->Type,(int)EventRelay1->State);
      EventRelay2->loop();
      if( EventRelay2->changeState()  )Serial.printf("!!! EventRelay2 Loop %d %d \n",(int)EventRelay2->Type,(int)EventRelay2->State);
      EventRGB1->loop();
      EventRGB2->loop();
      EventCalibrate->loop();
      EventBusy1->loop();
      EventBusy2->loop();
      vTaskDelay(250);
   }
}

/**
 * Задача работы с MP3
 * @param pvParameters
 */
void taskMP3(void *pvParameters) {
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! MP3 task start"));
#endif
   DFRobotDFPlayerMini myDFPlayer;
   EventMP3            = new TEventMP3(0,0,handleMP3,1,1,false);
   FPSerial.begin(9600, SERIAL_8N1, /*rx =*/PIN_RX1, /*tx =*/PIN_TX1);
   isDFPlayer = myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true);
   myDFPlayer.setTimeOut(500);
   myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
   myDFPlayer.volume(jsonConfig["MP3"]["VOLUNE"].as<int>());
   vTaskDelay(250);
   myDFPlayer.playFolder(1,123);
//   Serial.printf("!!! EventRelay1 Init %d %d \n",(int)EventRelay1->Type,(int)EventRelay1->State);
   while (true) {
      uint32_t ms = millis();
      switch( cmdMP3 ){
         case CMP3_VOLUME: myDFPlayer.volume(jsonConfig["MP3"]["VOLUNE"].as<int>()); cmdMP3 = CMP3_NONE; break;
         case CMP3_PLAY:   myDFPlayer.playFolder(arg1MP3, arg2MP3); cmdMP3 = CMP3_NONE; break;
         case CMP3_STOP:   myDFPlayer.stop(); cmdMP3 = CMP3_NONE; break;
      }
     
      if( ms1 == 0 || ms1 > ms || ms-ms1 > 2000 ){
         ms1 = ms;
         if( isPlayMP3 ){
            myDFPlayer.readState();
            int _stat = myDFPlayer.readState();
            if(  _stat == 0 ){
                Serial.printf("!!! Play MP3 completed %d\n",_stat);
//                EventMP3->off(0);
                if( EventMP3->Loop ){
                   EventMP3->reset();
                   EventMP3->on(2000);
                }
                isPlayMP3 = false;
            }
         }
      }
     
      EventMP3->loop();
      vTaskDelay(1000);
   }
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
   sensor = new MySensor();
   sensor->init();
   pinMode(PIN_OUT1,OUTPUT);
   setRelay1(statRelay1); 
   pinMode(PIN_OUT2,OUTPUT);
   setRelay1(statRelay2); 
   if( bootCount < 1 ){
      startCalibrate(jsonConfig["CALIBR"]["DELAY_START"].as<uint32_t>()*1000);
   }
   else {
      Distance = jsonSave["DISTANCE"].as<float>();
      SensorOn = (SENSOR_STAT_t)jsonSave["STATE_ON"].as<int>();
   }
   while (true) {
      uint32_t ms = millis();

      xSemaphoreTake(sensorSemaphore, portMAX_DELAY);
      uint16_t _status = (uint16_t)sensor->get();
      Distance = sensor->Value->getLast();
      if( calibrMode == CM_NONE ){
         if( isnan(Distance) ){ setNanMode(); }
         else {
            if( ( jsonConfig["SENSOR"]["INSTALL"].as<int>() == INSTALL_TYPE_NORMAL &&
                abs(jsonConfig["SENSOR"]["DIST_GROUND"].as<float>() - Distance ) < jsonConfig["SENSOR"]["DIST_LIMIT"].as<float>() )|| 
                ( jsonConfig["SENSOR"]["INSTALL"].as<int>() == INSTALL_TYPE_OUTSIDE &&
                ( (Distance < jsonConfig["SENSOR"]["DIST_MIN1"].as<float>() )||(Distance > jsonConfig["SENSOR"]["DIST_MAX1"].as<float>() )))||
                ( jsonConfig["SENSOR"]["INSTALL"].as<int>() == INSTALL_TYPE_INSIDE &&
                ( (Distance < jsonConfig["SENSOR"]["DIST_MIN2"].as<float>() )||(Distance > jsonConfig["SENSOR"]["DIST_MAX2"].as<float>() )))){            
               SensorOn = SS_FREE;
            }
            else { SensorOn = SS_BUSY;  }
            isChangeNan = true;
         }
         checkChangeOn();
      }
      else if( calibrMode == CM_ON){ //Режим калибровки
         if( calibrCount >= jsonConfig["CALIBR"]["NUMBER"].as<int>() )EventCalibrate->off();
         Serial.printf("!!! Calibrate %d %d\n",(int)Distance,calibrCount);
         if( !isnan(Distance) ){
             calibrAvg += Distance;
             calibrCount++;
         }
      }
      if( ms0 == 0 || ms0 > ms || ms-ms0 > 5000 ){
         ms0 = millis();
         printStat("TM");
      } 

      if( isChangeConfig ){
          isChangeConfig = false;
          ledBrightness( jsonConfig["RGB1"]["BRIGHTNESS"].as<int>() );
          led2Brightness( jsonConfig["RGB2"]["BRIGHTNESS"].as<int>() );
          void setVolumeMP3();
      }
      
      xSemaphoreGive(sensorSemaphore);
      vTaskDelay((uint32_t)(jsonConfig["SENSOR"]["T_LOOP"].as<float>()*1000));
  }
}

/**
* Обработччик события срабатывания сенсора
*/
void handleSensor(bool _flag){
#if defined(DEBUG_SERIAL)
   Serial.printf("!!! EventSensor %d %d %d\n",(int)_flag,(int)EventSensor->Type,(int)EventSensor->State);
#endif
   if( _flag ){
      EventRelay1->setType((TEVENT_TYPE_t)jsonConfig["RELAY1"]["MODE"].as<int>(),(uint32_t)(jsonConfig["RELAY1"]["T_PULSE"].as<float>()*1000),(uint32_t)(jsonConfig["RELAY1"]["T_PAUSE"].as<float>()*1000));
      EventRelay2->setType((TEVENT_TYPE_t)jsonConfig["RELAY2"]["MODE"].as<int>(),(uint32_t)(jsonConfig["RELAY2"]["T_PULSE"].as<float>()*1000),(uint32_t)(jsonConfig["RELAY2"]["T_PAUSE"].as<float>()*1000));
      EventRelay1->on();
      EventRelay2->on();
   }
   else {
      EventRelay1->off();
      EventRelay2->off();
   }
}

/**
* Обработчик события включения/выключение Реле1
*/
void handleRelay1(bool _flag){
#if defined(DEBUG_SERIAL)
   Serial.print(F("!!! EventsRelay1 "));
   Serial.println((int)_flag);
#endif
   setRelay1(_flag);
}

/**
* Обработчик события включения/выключение Реле2
*/
void handleRelay2(bool _flag){
#if defined(DEBUG_SERIAL)
   Serial.print(F("!!! EventsRelay2 "));
   Serial.println((int)_flag);
#endif
   setRelay2(_flag);
}

/**
* Обработчик события работы с RGB1
*/
void handleRGB1(bool _flag){
#if defined(DEBUG_SERIAL)
///   Serial.print(F("!!! EventsRGB1 "));
///   Serial.println((int)_flag);
#endif
   if( _flag ){
      if( EventRGB1->Type == ET_NORMAL && EventRGB1->Color2 != COLOR_NONE )ledSetColor2(EventRGB1->Color1,EventRGB1->Color2);
      else ledSetColor(EventRGB1->Color1);
   }
   else {
      if( EventRGB1->Type == ET_PWM )ledSetColor(EventRGB1->Color2);
   }
}

/**
* Обработчик события работы с RGB2
*/
void handleRGB2(bool _flag){
#if defined(DEBUG_SERIAL)
///   Serial.print(F("!!! EventsRGB2 "));
///   Serial.print(EventRGB2->Type);
///   Serial.print(" #");
///   Serial.print(EventRGB2->Color1,HEX);
///   Serial.print(" #");
///   Serial.print(EventRGB2->Color2,HEX);
///   Serial.print(' ');
///   Serial.println((int)_flag);
#endif
   if( _flag ){
      if( EventRGB2->Type == ET_NORMAL && EventRGB2->Color2 != COLOR_NONE )led2SetColor2(EventRGB2->Color1,EventRGB2->Color2);
      else led2SetColor(EventRGB2->Color1);
   }
   else {
      if( EventRGB2->Type == ET_PWM )led2SetColor(EventRGB2->Color2);
   }
}

/**
* Обработчик события работы с DFPlayer Mini
*/
void handleMP3(bool _flag){
#if defined(DEBUG_SERIAL)
   Serial.print(F("!!! EventsMP3 "));
   Serial.println((int)_flag);
#endif
   if( _flag ){
      playMP3(EventMP3->Dir, EventMP3->Sound);   
//      bool flag = isPlayMP3; 
      isPlayMP3 = true;
      if( EventMP3->ColorTM != 0 ){
//         if( flag )SaveRGB2->Restore(1);
         SaveRGB2->Save(1,ET_PWM,500,500,EventRGB2->Color1, jsonConfig["RGB2"]["MP3"].as<uint32_t>());
      }
      ms1 = millis();
   }
   else {
//      stopMP3();
//      isPlayMP3 = false;   
      if( EventMP3->ColorTM != 0 )SaveRGB2->Restore(1);                

   }
}

/**
* Обработчик события работы с RGB1
*/
void handleBusy1(bool _flag){
#if defined(DEBUG_SERIAL)
   Serial.print(F("!!! EventsBUSY1 "));
   Serial.println((int)_flag);
#endif
   if( _flag ){
      if( jsonConfig["MP3"]["BUSY1"]["ENABLE"].as<bool>() )setEventMP3(jsonConfig["MP3"]["BUSY1"],false);
   }
}

/**
* Обработчик события работы с RGB1
*/
void handleBusy2(bool _flag){
#if defined(DEBUG_SERIAL)
///   Serial.print(F("!!! EventsBUSY1 "));
///   Serial.println((int)_flag);
#endif
   if( _flag ){
      if( jsonConfig["MP3"]["BUSY2"]["ENABLE"].as<bool>() )setEventMP3(jsonConfig["MP3"]["BUSY2"],false);
   }
}



/**
* Начало калибровки через событие
*/
void startCalibrate(uint32_t _delay){
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! Calibrate Wait ... "));
#endif
   EventMP3->reset();
   EventCalibrate->setType(ET_PULSE,30000,0);
   EventCalibrate->on(_delay);
   calibrMode = CM_WAIT;
   SaveRGB1->Save(2,ET_PWM,250,250,COLOR_GROUND, COLOR_BLACK);
   SaveRGB2->Save(2,ET_PWM,250,250,COLOR_GROUND, COLOR_BLACK);
   isPlayMP3 = false;
}

/**
* Обработчик события калибровки
*/
void handleCalibrate(bool _flag){
#if defined(DEBUG_SERIAL)
   Serial.print(F("!!! Calibrate "));
   Serial.println((int)_flag);
#endif
   if( _flag ){
      calibrMode = CM_ON;
      calibrAvg = 0;
      calibrCount = 0;
      SaveRGB1->Restore(2);
      SaveRGB1->Save(2,ET_NORMAL,0,0,COLOR_GROUND, COLOR_NONE);
      SaveRGB2->Restore(2);
      SaveRGB2->Save(2,ET_NORMAL,0,0,COLOR_GROUND, COLOR_NONE);
      isPlayMP3 = false;
   }
   else {
      calibrMode   = CM_NONE;
      lastSensorOn = SS_NONE;
      if( calibrCount > 0 ){
          ledSetColor(COLOR_SAVE);
          led2SetColor(COLOR_SAVE);
          float x = calibrAvg/calibrCount;
          Serial.printf("!!! Calibrate ground value %d\n",(int)x);
          jsonConfig["SENSOR"]["DIST_GROUND"]  = (int)x;
          jsonConfig["SENSOR"]["DIST_MIN1"]    = (int)x;
          jsonConfig["SENSOR"]["DIST_MAX1"]    = (int)x;
          jsonConfig["SENSOR"]["DIST_MIN2"]    = (int)x;
          jsonConfig["SENSOR"]["DIST_MAX2"]    = (int)x;
          configSave(); 
          configRead();      
     }
     else {
          ledSetColor(COLOR_NAN);
          led2SetColor(COLOR_NAN);
     }
     SaveRGB1->Restore(2);
     SaveRGB2->Restore(2);
     vTaskDelay(250);

   }
}

/**
* Установка режима если датчик NAN
*/
void setNanMode(){
   switch( jsonConfig["RGB1"]["NAN_MODE"].as<int>() ){
      case NAN_VALUE_IGNORE: 
         SensorOn = SS_NAN;
         if(isChangeNan)Serial.println(F("!!! NAN. Skiping"));
         break;  
      case NAN_VALUE_BUSY:
         SensorOn = SS_BUSY;
         if(isChangeNan)Serial.println(F("!!! NAN. BUSY"));
         break;
      case NAN_VALUE_FREE:
         SensorOn = SS_FREE;
         if(isChangeNan)Serial.println(F("!!! NAN. FREE"));
         break;         
   }
   isChangeNan = false;
}

/*
* Установка эффекта на RGB1
*/
void setEventRGB1(TEVENT_TYPE_t _type, uint32_t _timeOn, uint32_t _timeOff, uint32_t _color1, uint32_t _color2){
    EventRGB1->setType(_type, _timeOn, _timeOff);
    EventRGB1->setColor(_color1, _color2);
    EventRGB1->reset();
    EventRGB1->on();   
}

/*
* Установка эффекта на RGB2
*/
void setEventRGB2(TEVENT_TYPE_t _type, uint32_t _timeOn, uint32_t _timeOff, uint32_t _color1, uint32_t _color2){
    if( isPlayMP3 )SaveRGB2->Clear();
    EventRGB2->setType(_type, _timeOn, _timeOff);
    EventRGB2->setColor(_color1, _color2);
    EventRGB2->reset();
    EventRGB2->on();   
    if( isPlayMP3 )SaveRGB2->Save(1,ET_PWM,500,500,EventRGB2->Color1, jsonConfig["RGB2"]["MP3"].as<uint32_t>());
}

/*
* Установка проигрывния звуковой дорожки
*/
void setEventMP3( bool _enable, uint32_t _delayOn, int _dir, int _sound, bool _loop, uint32_t _color, uint32_t _tm){
    EventMP3->setSound(_dir, _sound, _loop, _color, _tm);
    stopMP3();
    EventMP3->off(0);
    if( _tm == 0 )EventMP3->setType(ET_PULSE, 500, 0 );
    else EventMP3->setType(ET_PULSE, _tm, 0);
    if(_enable)EventMP3->on(_delayOn);
}

/*
* Установка проигрывния звуковой дорожки из JSON Объекта
*/
void setEventMP3( JsonObject _config, bool is_delay){
    uint32_t _delay = 0;
    if( is_delay )_delay = _config["DELAY"].as<uint32_t>()*1000;
    setEventMP3(_config["ENABLE"].as<bool>(),_delay,_config["DIR"].as<int>(),_config["NUM"].as<int>(),_config["LOOP"].as<bool>(),_config["COLOR"].as<uint32_t>(),_config["COLOR_TM"].as<uint32_t>()*1000 );
}

/*
* Проверка состояния сенсора и формирование нудный событий
*/
void checkChangeOn(){
   if( SensorOn == lastSensorOn )return;
   Serial.printf("!!! Set change stat %d\n", (int)SensorOn);
   switch(SensorOn){
      case SS_BUSY:
      case SS_NAN_BUSY:   
         EventSensor->on();
         EventBusy1->on(jsonConfig["MP3"]["BUSY1"]["DELAY"].as<uint32_t>()*1000);
         EventBusy2->on(jsonConfig["MP3"]["BUSY2"]["DELAY"].as<uint32_t>()*1000);
         setEventRGB1( ET_NORMAL, 0, 0, jsonConfig["RGB1"]["BUSY"].as<uint32_t>(), COLOR_NONE );
         setEventRGB2( ET_NORMAL, 0, 0, jsonConfig["RGB2"]["BUSY"].as<uint32_t>(), COLOR_NONE );
         setEventMP3(jsonConfig["MP3"]["BUSY"]);
         break;
      case SS_FREE:   
      case SS_NAN_FREE:   
         EventSensor->off();
         EventBusy1->reset();
         EventBusy2->reset();
         if( jsonConfig["RGB1"]["IS_FREE_BLINK"].as<bool>() )setEventRGB1( ET_PWM, 5000, 250, jsonConfig["RGB1"]["FREE"].as<uint32_t>(), jsonConfig["RGB1"]["FREE_BLINK"].as<uint32_t>() );
         else setEventRGB1( ET_NORMAL, 0, 0, jsonConfig["RGB1"]["FREE"].as<uint32_t>(), COLOR_NONE );  
         if( jsonConfig["RGB2"]["IS_FREE_BLINK"].as<bool>() )setEventRGB2( ET_PWM, 5000, 250, jsonConfig["RGB2"]["FREE"].as<uint32_t>(), jsonConfig["RGB2"]["FREE_BLINK"].as<uint32_t>() );
         else setEventRGB2( ET_NORMAL, 0, 0, jsonConfig["RGB2"]["FREE"].as<uint32_t>(), COLOR_NONE );  
         setEventMP3(jsonConfig["MP3"]["FREE"]);
         break;
      case SS_NAN:
         if(jsonConfig["RGB1"]["IS_NAN_MODE"].as<bool>()){
            switch(jsonConfig["RGB1"]["NAN_MODE"].as<int>()){
               case NAN_VALUE_IGNORE: 
                  setEventRGB1( ET_NORMAL, 0, 0, COLOR_NAN, COLOR_NONE); 
                  setEventRGB2( ET_NORMAL, 0, 0, COLOR_NAN, COLOR_NONE); 
//                  setEventRGB1( ET_NORMAL, 0, 0, COLOR_NONE, COLOR_NAN); 
//                  setEventRGB2( ET_NORMAL, 0, 0, COLOR_NONE, COLOR_NAN); 
                  break;
               case NAN_VALUE_BUSY:   
                  setEventRGB1( ET_NORMAL, 0, 0, jsonConfig["RGB1"]["BUSY"].as<uint32_t>(), COLOR_NAN); 
                  setEventRGB2( ET_NORMAL, 0, 0, jsonConfig["RGB2"]["BUSY"].as<uint32_t>(), COLOR_NAN); 
                  break;
               case NAN_VALUE_FREE:   
                  setEventRGB1( ET_NORMAL, 0, 0, jsonConfig["RGB1"]["FREE"].as<uint32_t>(), COLOR_NAN); 
                  setEventRGB2( ET_NORMAL, 0, 0, jsonConfig["RGB2"]["FREE"].as<uint32_t>(), COLOR_NAN); 
                  break;
            } 
         }
/*         
         if(jsonConfig["RGB2"]["IS_NAN_MODE"].as<bool>()){
            switch(jsonConfig["RGB2"]["NAN_MODE"].as<int>()){
               case NAN_VALUE_IGNORE: setEventRGB2( ET_NORMAL, 0, 0, COLOR_NONE, COLOR_NAN); break;
               case NAN_VALUE_BUSY:   setEventRGB2( ET_NORMAL, 0, 0, jsonConfig["RGB2"]["BUSY"].as<uint32_t>(), COLOR_NAN); break;
               case NAN_VALUE_FREE:   setEventRGB2( ET_NORMAL, 0, 0, jsonConfig["RGB2"]["FREE"].as<uint32_t>(), COLOR_NAN); break;
            } 
         }
         */
         if( lastSensorOn == SS_FREE )setEventMP3(jsonConfig["MP3"]["FREE_NAN"]);
         else setEventMP3(jsonConfig["MP3"]["NAN"]);
         break;
   }
   lastSensorOn = SensorOn;
   isPlayMP3 = false;
   msSendHttp = millis();
   saveSet(Distance,SensorOn);
}

/*
* Установка состояние реле1
*/
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

/*
* Установка состояние реле2
*/
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


/*
* Установка состояния физического GPIO пина реле
*/
void  setRelayPin(uint8_t pin, bool stat, bool is_inverse){
   bool _stat;
   if( is_inverse )_stat = !stat;
   else _stat = stat;
//   Serial.printf("!!! Set Relay pin=%d stat=%d\n",(int)pin,(int)_stat);
   digitalWrite(pin, _stat);
}

/*
* Вывод на экран состояния сенсора
*/
void printStat(char *msg){
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
/*
float CalibrateGround(){
   uint32_t ms_start = millis();
// Макимальное ограничение калиброка 30сек
   for( uint32_t ms=millis(); ms-ms_start<5000; ms=millis()){
// Делаем цикл измерения
      int n = 0;
      float distArray[jsonConfig["CALIBR"]["NUMBER"].as<int>()];
      float distAvg = 0, distDiv = 0;
      for( int i=0; i<jsonConfig["CALIBR"]["NUMBER"].as<int>(); i++){
         uint16_t _status = (uint16_t)sensor->get();
         float L = sensor->Value->getLast();
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
*/

/*
* Задача работы с кнопкой (герконом)
*/
void taskButton(void *pvParameters){
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! Button task start"));
#endif
   pinMode(PIN_BTN,INPUT);
   SBTN btn(PIN_BTN);
   btn.setTimer(10000);
   btn.setTimerEventCount(4000);
   bool white_flag = false;
   uint32_t tm;
   int btn_count;
   while(true){
      switch(btn.loop()){
          case SB_PRESS:
             btn_count = (int)btn.getCountEvent();
             Serial.printf("!!! BTN Press %d\n",btn_count);
             SaveRGB1->Save(3,ET_PWM,500,500,COLOR_GROUND, COLOR_BLACK);
             SaveRGB2->Save(3,ET_PWM,500,500,COLOR_GROUND, COLOR_BLACK);
             if( btn_count == 5 ){
                ledSetColor(COLOR_ERROR);
                btn_count = 0;
                jsonConfig["SYSTEM"]["NAME"]        = DEVICE_NAME;
                configSave();     
                delay(1000);    
                ESP.restart();  
             }
             break;
          case SB_RELEASE:
            SaveRGB1->Restore(3);
            SaveRGB2->Restore(3);
             tm = btn.getPressTime();
             if( tm >= 2000 && tm < 10000 ){
                 startCalibrate(jsonConfig["CALIBR"]["DELAY_START"].as<uint32_t>()*1000);
                 if( bootCount>=1 ){
                    if( isAP )isAP = false;
                    else isAP = true; 
 //                   Serial.printf("!!! WiFi is %d\n",(int)isAP);
                 }
             }
             Serial.printf("!!! BTN Release %d\n",(int)tm);
             break;
          case SB_TIMER:
            SaveRGB1->Restore(3);
            SaveRGB2->Restore(3);
             Serial.printf("!!! BTN Timer %d\n",(int)btn.getPressTime());
             ledSetColor(COLOR_ERROR);
             jsonSave["BOOT_COUNT"]  = 0;
             saveSave();
             jsonConfig["SYSTEM"]["PASS0"]       = DEVICE_PASS0;               //Пароль суперадминистратора
             jsonConfig["SYSTEM"]["PASS1"]       = DEVICE_PASS1;               //Пароль администратора
             configSave();
             delay(1000);    
             ESP.restart();  
             break;
      }
       vTaskDelay(200);
       
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
   WiFi.mode(WIFI_OFF);
   uint32_t ms2 = 0, ms3 = 0;
   ledSetColorAP(COLOR_BLACK);
   Network.onEvent(handleEventWiFi);
   if( jsonConfig["SYSTEM"]["AP_START"].as<bool>() ||  bootCount<1 )isAP = true;
   else isAP = false;
//   HTTP_begin();
   while(true){
      uint32_t ms = millis();
      if( ms2 == 0 || ms < ms2 || (ms-ms2)>3000){
          ms2 = ms;
          wifi_mode_t curWiFi = WiFi.getMode(); 
          if( jsonConfig["CRM_MOSCOW"]["ENABLE"].as<bool>() == true && !jsonConfig["WIFI"]["NAME"].isNull() )isSTA = true;
          else isSTA = false;
// Стартуем точку доступа          
          if( isAP && ( curWiFi != WIFI_AP && curWiFi != WIFI_AP_STA) ){
             WiFi_ScanNetwork();
             WiFi.enableAP(true);
             WiFi.softAP(jsonConfig["SYSTEM"]["NAME"].as<String>());
             HTTP_begin();
             msAP = millis();                
          }
// Гасим точку доступа
          if( !isAP && ( curWiFi == WIFI_AP || curWiFi == WIFI_AP_STA) ){
//             Serial.println(F("!!! Disable AP"));
             WiFi.enableAP(false);
          }
          if( isSTA && ( curWiFi != WIFI_STA && curWiFi != WIFI_AP_STA) ){
             Serial.println(F("!!! Start STA"));
             msSTA = ms;
             WiFi_ScanNetwork();
             WiFi.enableSTA(true);
             if( jsonConfig["WIFI"]["DHCP"].as<bool>() == false ){
                IPAddress ip_addr,ip_mask,ip_gate,ip_dns;
                if( ip_addr.fromString( jsonConfig["WIFI"]["IP"]["ADDR"].as<String>() ) &&
                    ip_mask.fromString( jsonConfig["WIFI"]["IP"]["MASK"].as<String>() ) &&
                    ip_gate.fromString( jsonConfig["WIFI"]["IP"]["GW"].as<String>() ) &&
                    ip_dns.fromString(  jsonConfig["WIFI"]["IP"]["DNS"].as<String>()     ) ){
                   Serial.println("Config static IP address");
                   WiFi.config(ip_addr,ip_gate,ip_mask,ip_dns);
                }                 
             }
             WiFi.begin(jsonConfig["WIFI"]["NAME"].as<String>(), jsonConfig["WIFI"]["PASS"].as<String>());
             ledSTA(true);
             ledSetColorSTA(COLOR_WIFI_WAIT);
          }      
          if( isSTA && ( curWiFi == WIFI_STA || curWiFi == WIFI_AP_STA) ){
             if( WiFi.status() != WL_CONNECTED && (ms - msSTA)>10000 ){
                 Serial.println(F("!!! Error WiFi"));
                 ledSetColorSTA(COLOR_WIFI_OFF);          
                 WiFi.enableSTA(false);
             }
          }
          if( !isSTA && ( curWiFi == WIFI_STA || curWiFi == WIFI_AP_STA) ){
             WiFi.enableSTA(false);
             ledSTA( false );
             ledSetColorSTA(COLOR_BLACK);          
          }

// Стартуем подключение

/*
          wifi_mode_t modeWiFi =  WIFI_OFF;
          if( isAP && isSTA )modeWiFi = WIFI_AP_STA;
          else if( isAP )modeWiFi     = WIFI_AP;
          else if( isSTA )modeWiFi    = WIFI_STA; 
*/

      }
      if( (ms3 == 0 || ms < ms3 || (ms-ms3)>2000) ){
          ms3 = ms;
          if( msSendHttp < ms ){
             msSendHttp = 0;
             uint32_t ms_tmp = 0;
             if( sendHttpParam() )ms_tmp = ms + jsonConfig["CRM_MOSCOW"]["T_SEND"].as<uint32_t>()*1000;
             else ms_tmp = ms + jsonConfig["CRM_MOSCOW"]["T_RETRY"].as<uint32_t>()*1000;
             if( msSendHttp == 0)msSendHttp = ms_tmp;
          }
      }
      HTTP_loop();
      vTaskDelay(50);      
   }


}

void handleEventWiFi(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:     Serial.println("STA Started"); break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED: Serial.println("STA Connected"); break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      ledSetColorSTA(COLOR_WIFI_ON);          
      Serial.println("!!! STA Got IP");
      Serial.println(WiFi.STA);
//      WiFi.AP.enableNAPT(true);
      break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      Serial.println("!!! STA Lost IP");
      ledSetColorSTA(COLOR_WIFI_WAIT);          
      msSTA = millis();
//      WiFi.AP.enableNAPT(false);
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("!!! STA Disconnected");
//      WiFi.AP.enableNAPT(false);
      break;
    case ARDUINO_EVENT_WIFI_STA_STOP: Serial.println("STA Stopped"); break;

    case ARDUINO_EVENT_WIFI_AP_START:
//      w_stat2 = EWS_AP_MODE;
      if( bootCount<0 )ledSetColorAP(COLOR_WIFI_AP);
      else ledSetColorAP(COLOR_WIFI_AP1);
      Serial.println("AP Started");
      Serial.println(WiFi.AP);
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:    Serial.println("AP STA Connected"); break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED: Serial.println("AP STA Disconnected"); break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      Serial.print("AP STA IP Assigned: ");
      Serial.println(IPAddress(info.wifi_ap_staipassigned.ip.addr));
      break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED: Serial.println("AP Probe Request Received"); break;
    case ARDUINO_EVENT_WIFI_AP_STOP:       
      Serial.println("AP Stopped"); 
      ledSetColorAP(COLOR_BLACK);
      break;

    default: break;
  }
}

bool sendHttpParam(){
   bool ret = false;
   String str = "";
   str += "http://";
   str += jsonConfig["CRM_MOSCOW"]["SERVER"].as<String>();
   str += ":";
   str += jsonConfig["CRM_MOSCOW"]["PORT"].as<int>();
   str += HTTP_PATH;
   str += "?id=";
   str += jsonConfig["CRM_MOSCOW"]["DOGOVOR_ID"].as<String>();
   str += "_";
   str += jsonConfig["CRM_MOSCOW"]["BOX_ID"].as<String>();
   str += "&temp=0&hum=0&dist=";
   str += String(Distance,0);
   str += "&tm=";
   str += String(millis()/1000);
   str += "&btn=";
   switch(SensorOn){
      case SS_BUSY:
      case SS_NAN_BUSY: str += "1";break;
      case SS_FREE:   
      case SS_NAN_FREE: str += "1";break;  
      default: str += "-1";
   }   
   str += "&uptime=";
   str += String(millis()/1000);

//   httpClient.begin(jsonConfig["CRM_MOSCOW"]["SERVER"].as<String>(), jsonConfig["CRM_MOSCOW"]["PORT"].as<int>(),str);
   Serial.println(str);
   httpClient.begin(str);
   int httpCode = httpClient.GET();
   httpClient.end();
   Serial.print(F("!!! HTTP send "));
   Serial.println(jsonConfig["CRM_MOSCOW"]["SERVER"].as<String>());
   
   if( httpCode == HTTP_CODE_OK ){
        String payload = httpClient.getString();
        Serial.print(" success: ");
        Serial.println(payload);
        ret = true;
   }
   else {
        Serial.print(" error: ");
        Serial.println(httpCode);
   }
//   sprintf(sbuf,"GET http://%s:%d%s?id=%s&temp=%d&hum=%d&dist=%d&tm=%ld&btn=%d&uptime=%ld&key=%d HTTP/1.0\r\n\r\n",
//      EA_Config.SERVER,EA_Config.PORT,HTTP_PATH,SensorID,_temp,_hum,
//      _dist,_time,(int)_btn,_uptime,(int)KeyGen());
   return ret;    
}

/**
 * Генерация контрольной суммы 
 */
 /*
uint16_t KeyGen(){
   sprintf_P(rbuf,KeyGenStrParam,SensorID,Time,Distance,Time,Hum);
   uint16_t crc = 0;
   for( int i=0; i< strlen(rbuf); i++ ){
       crc += (int)rbuf[i];
   }    
   crc = ( ~ crc )&0xfff;   
   return crc;
  
}
*/

void setVolumeMP3(){
    cmdMP3  = CMP3_VOLUME;
 }

void playMP3(int _dir, int _num){
    cmdMP3  = CMP3_PLAY;
    arg1MP3 = _dir;
    arg2MP3 = _num;
    
}
void stopMP3(){
    cmdMP3  = CMP3_STOP;
}

