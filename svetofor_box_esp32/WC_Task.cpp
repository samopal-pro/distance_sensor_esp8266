#include "WC_Task.h"
char strID[16];
uint64_t chipID;
char serNo[33];

int MP3_ADD_DIR = MP3_SYSTEM_FULL_DIR;

MySensor *sensor;
TEvent *EventSensor, *EventRelay1, *EventRelay2;
TEvent *EventBusy1, *EventBusy2;
TEventRGB *EventRGB1, *EventRGB2, *SaveRGB1, *SaveRGB2;
//TEventRGB  *SaveEventRGB2, *SaveEventCalibrate1, *SaveEventCalibrate2;
//TSaveRGB *SaveRGB1, *SaveRGB2;


TEventMP3 *EventMP3 = NULL;
TEvent *EventCalibrate; 

//bool isDFPlayer = false;
bool isPlayMP3  = false;
//CMD_MP3_t cmdMP3 = CMP3_NONE;
//int arg1MP3 = 0, arg2MP3 = 0;


float Distance = NAN, lastDistance = NAN;
SENSOR_STAT_t SensorOn = SS_NONE, lastSensorOn = SS_NONE;  
ES_STAT stat1 = STAT_OFF, stat2 = STAT_OFF;
uint32_t msStat1  = 0, msStat2 = 0;
bool statRelay1 = false, statRelay2 = false;
bool inverseRelay1 = false, inverseRelay2 = false;
uint16_t eventRelay1 = 0, eventRelay2 = 0;
uint32_t msRelay1 = 0, msRelay2 = 0;

char calibrCheck[5], calibrNum = -1;

CALIBRATION_MODE_t calibrMode = CM_NONE;
float calibrAvg = 0;
uint16_t calibrCount = 0, calibrError = 0;

uint32_t ms0 = 0, ms1 = 0;

bool isSensorBlock = false; //Полная блокировка сенсора до перезагрузки. Работают только звук и свет

bool isChangeNan   = true;
bool isChangeStat  = false; //Изменение отслеживания изменения состояния для немедленной отправки 
uint16_t bootCount;
SemaphoreHandle_t sensorSemaphore, soundSemaphore/*, bootSemaphore */;
uint32_t colorMP3   = COLOR_SAVE;

bool isSendNet = false; // Флаг отсылки парамтров на сервер



//typedef struct {
//    union {
//       uint32_t data32;
//       uint8_t  data8[4];
//    };
//} uint32_to_uint8;

    union uint32_to_uint8 {
       uint32_t data32;
       char  data8[4];
    };



/**
 * Старт всех параллельных задач
 */
void tasksStart() {

  configInit();
//  configDefault();
//  configSave();
  ;
  configRead();
  saveRead();
  bootCount = saveCount(); 
  chipID = ESP.getEfuseMac();
  sprintf(strID,"%012llX",chipID);
// Считываем серийник с efuse
// Прошивать SAV00001 espefuse  -c auto -p COM3 burn_efuse BLOCK3 0x5341563030303031000000000000000000000000000000000000000000000000  
  uint32_to_uint8 dd;
  for( int i=0; i<8; i++){
     dd.data32 = esp_efuse_read_reg(EFUSE_BLK3, 7-i);
     for( int j=0; j<4; j++)serNo[i*4+j] = dd.data8[3-j];
  }
  serNo[32] = '\0';
  Serial.printf("!!! ID=%s SER=%s ID1=",strID,serNo);
  Serial.println(chipID,HEX);
  
  //    xTaskCreateUniversal(taskLed, "led", 2048, NULL, 2, NULL,CORE);
  sensorSemaphore = xSemaphoreCreateMutex();
  soundSemaphore  = xSemaphoreCreateMutex();
//  bootSemaphore   = xSemaphoreCreateMutex();

   EventSensor         = new TEvent(0,0,handleSensor);
   EventRelay1         = new TEvent((uint32_t)(jsonConfig["RELAY1"]["DELAY_ON"].as<float>()*1000),(uint32_t)(jsonConfig["RELAY1"]["DELAY_OFF"].as<float>()*1000),handleRelay1);
   EventRelay2         = new TEvent((uint32_t)(jsonConfig["RELAY2"]["DELAY_ON"].as<float>()*1000),(uint32_t)(jsonConfig["RELAY2"]["DELAY_OFF"].as<float>()*1000),handleRelay2);
   EventBusy1          = new TEvent(jsonConfig["MP3"]["BUSY1"]["DELAY"].as<uint32_t>(),0,handleBusy1);
   EventBusy2          = new TEvent(jsonConfig["MP3"]["BUSY2"]["DELAY"].as<uint32_t>(),0,handleBusy2);
   EventCalibrate      = new TEvent(0,0,handleCalibrate);

   EventRGB1           = new TEventRGB(PIN_RGB1, COUNT_RGB1, 2);
   EventRGB2           = new TEventRGB(PIN_RGB2, COUNT_RGB2, 0);
//   SaveRGB1            = new TEventRGB(-1, 0, 0);
//   SaveRGB2            = new TEventRGB(-1, 0, 0);
   EventRGB1->setBrightness( jsonConfig["RGB1"]["BRIGHTNESS"].as<int>() );
   EventRGB2->setBrightness( jsonConfig["RGB2"]["BRIGHTNESS"].as<int>() );

   isPlayMP3 = true;
   FPSerial.begin(9600, SERIAL_8N1, PIN_RX1, PIN_TX1);
   EventMP3 = new TEventMP3(Serial1,handleMP3);
   EventMP3->setVolume(jsonConfig["MP3"]["VOLUME"].as<int>());
   if( jsonConfig["MP3"]["SHORT_MSG"].as<bool>() )MP3_ADD_DIR = MP3_SYSTEM_SHORT_DIR; 
   else MP3_ADD_DIR = MP3_SYSTEM_FULL_DIR; 

   xTaskCreateUniversal(taskEvents, "events", 10000, NULL, 3, NULL, CORE);
   xTaskCreateUniversal(taskRGB, "rgb", 10000, NULL, 3, NULL, CORE);
   xTaskCreateUniversal(taskMP3, "mp3", 10000, NULL, 1, NULL, CORE);
   xTaskCreateUniversal(taskSensors, "sensors", 30000, NULL, 4, NULL, CORE);
   vTaskDelay(500);
   xTaskCreateUniversal(taskButton, "btn", 4096, NULL, 4, NULL,CORE);
   vTaskDelay(500);
   xTaskCreateUniversal(taskNet, "net", 10000, NULL, 3, NULL, CORE);
//  vTaskDelay(500);
//   xTaskCreateUniversal(taskLora, "lora", 10000, NULL, 2, NULL, CORE);
}

/**
 * Задача работы с событиями
 * @param pvParameters
 */
void taskEvents(void *pvParameters) {
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! Events task start"));
#endif

//   Serial.printf("!!! EventRelay1 Init %d %d \n",(int)EventRelay1->Type,(int)EventRelay1->State);
   while (true) {
      uint32_t ms = millis();
      EventSensor->loop();
//      if( EventSensor->changeState()  )Serial.printf("!!! EventSensor Loop %d %d \n",(int)EventSensor->Type,(int)EventSensor->State);
      EventRelay1->loop();
      if( EventRelay1->changeState()  )Serial.printf("!!! EventRelay1 Loop %d %d \n",(int)EventRelay1->Type,(int)EventRelay1->State);
      EventRelay2->loop();
      if( EventRelay2->changeState()  )Serial.printf("!!! EventRelay2 Loop %d %d \n",(int)EventRelay2->Type,(int)EventRelay2->State);
//      EventRGB1->loop();
//      EventRGB2->loop();
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
   while (true) {
      if( EventMP3->loop() == ES_NONE )isPlayMP3 = false;
      else isPlayMP3 = true;
      vTaskDelay(1000);
   }
}

/**
 * Задача работы с RGB
 * @param pvParameters
 */
void taskRGB(void *pvParameters) {
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! RGB task start"));
#endif
   uint32_t ms1 = 0;
   while (true) {
      uint32_t _ms = millis();
      TEVENT_RGB_TYPE_t t = EventRGB1->loop();
      EventRGB2->loop();
      if( t != ERT_RAINBOW )vTaskDelay(250);
      else vTaskDelay(25);
//      if( ms1 == 0 || ms1 > _ms || _ms-ms1 > 1000){
//          ms1 = _ms;
//      Serial.printf("!!! Rgb status %d %d\n", t, (int)EventRGB1->isRainbow);
//      }

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
   sensor = new MySensor();
   sensor->init();
   pinMode(PIN_OUT1,OUTPUT);
   setRelay1(statRelay1); 
   pinMode(PIN_OUT2,OUTPUT);
   setRelay1(statRelay2); 
//   checkPlayMP3("100",100);

// Стартовое приветсвие
   EventRGB1->setRainbow(true);
   EventRGB2->setRainbow(true);
   uint32_t _ms1 = millis();
   waitMP3(DEFAULT_TIMER_MP3);
   systemMP3("100", 100, PRIORITY_MP3_MAXIMAL );
   waitMP3(30000);
   uint32_t _ms2 = millis();
   if( _ms2 - _ms1 < 3000 )vTaskDelay( 3000 - (_ms2-_ms1) );
   EventRGB1->setRainbow(false);
   EventRGB2->setRainbow(false);

   EventRGB1->set(COLOR_NAN,COLOR_NAN);
   EventRGB2->set(COLOR_NAN,COLOR_NAN);

// Если самый первый старт
   if( bootCount < 1 ){
      vTaskDelay(1000);
      systemMP3("99",99,PRIORITY_MP3_MAXIMAL);
      startCalibrate(0,"97",97,CM_WAIT_WAIT);
//      vTaskDelay(jsonConfig["MP3"]["99"]["COLOR_TM"].as<uint32_t>()*1000);
//      startCalibrate(jsonConfig["CALIBR"]["DELAY_START"].as<uint32_t>()*1000);
   }
   else {
      Distance = jsonSave["DISTANCE"].as<float>();
      SensorOn = (SENSOR_STAT_t)jsonSave["STATE_ON"].as<int>();
   }

   while (true) {
      uint32_t ms = millis();
      if(isSensorBlock){ //Сенсор заблокирован до выключения питания
         Serial.println(F("!!! Sensor blocked. Wait reboot ..."));
         vTaskDelay(5000);    
         continue;
      }

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

// Режим калибровки
      else if( calibrMode == CM_ON){ 
         int calibrMax =  jsonConfig["CALIBR"]["NUMBER"].as<int>();
         if( calibrCount >= calibrMax )EventCalibrate->off();
         if( calibrCount + calibrError >= calibrMax*3 )EventCalibrate->off();
         Serial.printf("!!! Calibrate %d %d\n",(int)Distance,calibrCount);
         if( !isnan(Distance) ){
             calibrAvg += Distance;
             calibrCount++;
         }
         else {
             calibrError++;
         }
      }     
      else if( calibrMode == CM_WAIT_WAIT ){
         if( EventMP3->State == ES_NONE ){
            Serial.printf("!!! Wait clibrate");
            if( calibrNum >0 )systemMP3(calibrCheck,calibrNum,PRIORITY_MP3_HIGH);
            startCalibrate(0);
         } 

      }
      else if( calibrMode == CM_WAIT_MP3 ){
//         Serial.printf( "!!! Calibr Wait MP3 %d\n",EventMP3->State);
         if( EventMP3->State == ES_NONE ){
            Serial.printf("!!! Start clibrate");
            if( calibrNum >0 )systemMP3(calibrCheck,calibrNum,PRIORITY_MP3_HIGH);
            EventCalibrate->reset();
            EventCalibrate->setType(ET_NORMAL);
            EventCalibrate->on(0);            
         } 
      }
      else if( calibrMode == CM_WAIT_REBOOT ){
         if( EventMP3->State == ES_NONE ){
             ESP.restart();
         }
      }
      if( ms0 == 0 || ms0 > ms || ms-ms0 > 5000 ){
         ms0 = ms;
         printStat("TM");
      } 
// Если поменялая яркость лент и громкость звуков
      if( isChangeConfig ){
          isChangeConfig = false;
          EventRGB1->setBrightness( jsonConfig["RGB1"]["BRIGHTNESS"].as<int>() );
          EventRGB2->setBrightness( jsonConfig["RGB2"]["BRIGHTNESS"].as<int>() );
          EventMP3->setVolume(jsonConfig["MP3"]["VOLUME"].as<int>());
          if( jsonConfig["MP3"]["SHORT_MSG"].as<bool>() )MP3_ADD_DIR = MP3_SYSTEM_SHORT_DIR; 
          else MP3_ADD_DIR = MP3_SYSTEM_FULL_DIR; 
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
* Обработчик события работы с DFPlayer Mini
*/
void handleMP3(bool _flag){
#if defined(DEBUG_SERIAL)
   Serial.print(F("!!! EventsMP3 "));
   Serial.println((int)_flag);
#endif

   if( _flag ){
      EventRGB2->setMP3(EventMP3->Color);
   }
   else {
      EventRGB2->setMP3(COLOR_NONE);
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
      if( jsonConfig["MP3"]["BUSY1"]["ENABLE"].as<bool>() )baseMP3(jsonConfig["MP3"]["BUSY1"],false);
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
      if( jsonConfig["MP3"]["BUSY2"]["ENABLE"].as<bool>() )baseMP3(jsonConfig["MP3"]["BUSY2"],false);
   }
}



/**
* Начало калибровки через событие
*/
void startCalibrate(uint32_t _delay, char *_check, int _num, CALIBRATION_MODE_t _mode){
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! Calibrate Wait ... "));
#endif
//   EventMP3->reset();
   
   if( _check!= NULL)strncpy(calibrCheck, _check, 4);
   else calibrCheck[0] = '\0';
   calibrNum = _num;
   EventCalibrate->reset();
   calibrMode = _mode;
   if( _mode == CM_WAIT_MP3){
      EventRGB1->set(COLOR_GROUND,COLOR_GROUND,COLOR_BLACK,COLOR_BLACK,250,250);
      EventRGB2->set(COLOR_GROUND,COLOR_GROUND,COLOR_BLACK,COLOR_BLACK,250,250);
   }
//   systemMP3("97",97,PRIORITY_MP3_HIGH);
//   waitMP3();

//   EventCalibrate->reset();
//   EventCalibrate->setType(ET_PULSE,30000,0);
//   EventCalibrate->on(_delay);
//   isPlayMP3 = false;
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
      Serial.println("!!! Start calibr");
      calibrMode = CM_ON;
      calibrAvg = 0;
      calibrCount = 0;
      calibrError = 0;
      EventRGB1->set(COLOR_GROUND,COLOR_GROUND);
      EventRGB2->set(COLOR_GROUND,COLOR_GROUND);
      isPlayMP3 = false;
   }
   else {
      Serial.println("!!! Stop calibr");
      if( calibrCount > 0 ){
          bool ret = false;
          if(calibrError < calibrCount)systemMP3("97",93,PRIORITY_MP3_HIGH);
          else systemMP3("97",94,PRIORITY_MP3_HIGH);
          EventRGB1->set(COLOR_SAVE,COLOR_SAVE);
          EventRGB2->set(COLOR_SAVE,COLOR_SAVE);
          float x = calibrAvg/calibrCount;
          Serial.printf("!!! Calibrate ground value %d\n",(int)x);
          jsonConfig["SENSOR"]["DIST_GROUND"]  = (int)x;
          jsonConfig["SENSOR"]["DIST_MIN1"]    = (int)x;
          jsonConfig["SENSOR"]["DIST_MAX1"]    = (int)x;
          jsonConfig["SENSOR"]["DIST_MIN2"]    = (int)x;
          jsonConfig["SENSOR"]["DIST_MAX2"]    = (int)x;
          configSave(); 
          configRead();
//          if( ret )vTaskDelay(3000);      
     }
     else {
          Serial.println("!!! cal1");
          systemMP3("97",95,PRIORITY_MP3_HIGH);
          Serial.println("!!! cal2");
          EventRGB1->set(COLOR_NAN,COLOR_NAN);
          EventRGB2->set(COLOR_NAN,COLOR_NAN);
     }
     calibrMode   = CM_NONE;
     lastSensorOn = SS_NONE;

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
         SensorOn = SS_NAN;
         if(isChangeNan)Serial.println(F("!!! NAN. BUSY"));
         break;
      case NAN_VALUE_FREE:
         SensorOn = SS_NAN;
         if(isChangeNan)Serial.println(F("!!! NAN. FREE"));
         break;         
   }
   isChangeNan = false;
}



/*
* Проверка состояния сенсора и формирование нудный событий
*/
void checkChangeOn(){
   uint32_t _color1, _color2;
   if( SensorOn == lastSensorOn )return;
   Serial.printf("!!! Stat is change %d %d\n", (int)SensorOn,(int)lastSensorOn);
   isSendNet = true;
   switch(SensorOn){
      case SS_BUSY:
//      case SS_NAN_BUSY:   
         if(lastSensorOn!=SS_RESTORE){   
            EventSensor->on();
            if(EventBusy1->State != ES_WAIT_ON && EventBusy1->State != ES_ON)EventBusy1->on(jsonConfig["MP3"]["BUSY1"]["DELAY"].as<uint32_t>()*1000);
            if(EventBusy2->State != ES_WAIT_ON && EventBusy2->State != ES_ON)EventBusy2->on(jsonConfig["MP3"]["BUSY2"]["DELAY"].as<uint32_t>()*1000);
            if(lastSensorOn != SS_NAN) baseMP3(jsonConfig["MP3"]["BUSY"]);
         }
         EventRGB1->set(jsonConfig["RGB1"]["BUSY"].as<uint32_t>(),jsonConfig["RGB1"]["BUSY"].as<uint32_t>());
         EventRGB2->set(jsonConfig["RGB2"]["BUSY"].as<uint32_t>(),jsonConfig["RGB2"]["BUSY"].as<uint32_t>());
         break;
      case SS_FREE:   
//      case SS_NAN_FREE:
         if(lastSensorOn!=SS_RESTORE){   
            EventSensor->off();
            EventBusy1->reset();
            EventBusy2->reset();
            baseMP3(jsonConfig["MP3"]["FREE"]);
         }
         if( jsonConfig["RGB1"]["IS_FREE_BLINK"].as<bool>() )EventRGB1->set(jsonConfig["RGB1"]["FREE"].as<uint32_t>(),jsonConfig["RGB1"]["FREE"].as<uint32_t>(),jsonConfig["RGB1"]["FREE_BLINK"].as<uint32_t>(),jsonConfig["RGB1"]["FREE_BLINK"].as<uint32_t>(),5000,250);
         else EventRGB1->set(jsonConfig["RGB1"]["FREE"].as<uint32_t>(),jsonConfig["RGB1"]["FREE"].as<uint32_t>() );  
         if( jsonConfig["RGB2"]["IS_FREE_BLINK"].as<bool>() )EventRGB2->set(jsonConfig["RGB2"]["FREE"].as<uint32_t>(),jsonConfig["RGB2"]["FREE"].as<uint32_t>(),jsonConfig["RGB2"]["FREE_BLINK"].as<uint32_t>(),jsonConfig["RGB2"]["FREE_BLINK"].as<uint32_t>(),5000,250);
         else EventRGB2->set(jsonConfig["RGB2"]["FREE"].as<uint32_t>(),jsonConfig["RGB2"]["FREE"].as<uint32_t>() );  
         break;
      case SS_NAN:
      case SS_NAN_FREE:   
      case SS_NAN_BUSY:   
         if(jsonConfig["RGB1"]["IS_NAN_MODE"].as<bool>()){
            switch(jsonConfig["RGB1"]["NAN_MODE"].as<int>()){
               case NAN_VALUE_IGNORE: 
                  _color1 = EventRGB1->Color1;
                  _color2 = EventRGB2->Color1;
//                  setEventRGB1( ET_NORMAL, 0, 0, COLOR_NAN, COLOR_NONE); 
//                  setEventRGB2( ET_NORMAL, 0, 0, COLOR_NAN, COLOR_NONE); 
                  EventRGB1->set( _color1, COLOR_NAN );  
                  EventRGB2->set( _color2, COLOR_NAN );  
                  break;
               case NAN_VALUE_BUSY:   
                  EventRGB1->set( jsonConfig["RGB1"]["BUSY"].as<uint32_t>(), COLOR_NAN); 
                  EventRGB2->set( jsonConfig["RGB2"]["BUSY"].as<uint32_t>(), COLOR_NAN); 
                  break;
               case NAN_VALUE_FREE:   
                  EventRGB1->set( jsonConfig["RGB1"]["FREE"].as<uint32_t>(), COLOR_NAN); 
                  EventRGB2->set( jsonConfig["RGB2"]["FREE"].as<uint32_t>(), COLOR_NAN); 
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
         if(lastSensorOn==SS_NONE){
            systemMP3("97",95,PRIORITY_MP3_HIGH);
         }
         else if(lastSensorOn!=SS_RESTORE){   
            if( lastSensorOn == SS_FREE )baseMP3(jsonConfig["MP3"]["FREE_NAN"]);
            else baseMP3(jsonConfig["MP3"]["NAN"]);
         }
         break;
   }
   lastSensorOn = SensorOn;
   isPlayMP3 = false;
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
   Serial.print(msg);
   Serial.print(F(" Free mem: "));
   Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
#endif
}


/*
* Задача работы с кнопкой (герконом)
*/
void taskButton(void *pvParameters){
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! Button task start"));
#endif
   pinMode(PIN_BTN,INPUT);
   SBTN btn(PIN_BTN);
   btn.setTimer(2000);
   btn.setTimer2(10000);
   btn.setTimerEventCount(4000);
   bool white_flag = false;
   bool is_early = false;
   uint32_t tm;
   int btn_count;
   while(true){
      if(isSensorBlock || calibrMode == CM_WAIT_REBOOT ){ //Сенсор заблокирован до выключения питания
         vTaskDelay(1000);    
         continue;
      }


      switch(btn.loop()){
          case SB_PRESS:
             is_early = false;
             btn_count = (int)btn.getCountEvent();
             Serial.printf("!!! BTN Press %d\n",btn_count);
//             EventRGB1->copyTo(SaveRGB1);
//             EventRGB2->copyTo(SaveRGB2);
             EventRGB1->set(COLOR_SAVE,COLOR_SAVE,COLOR_BLACK,COLOR_BLACK,250,250);               
             EventRGB2->set(COLOR_SAVE,COLOR_SAVE,COLOR_BLACK,COLOR_BLACK,250,250);               
             if( btn_count == 5 ){
                systemMP3("92",92,PRIORITY_MP3_HIGH);
                EventRGB1->set(COLOR_SAVE,COLOR_SAVE);               
                EventRGB2->set(COLOR_SAVE,COLOR_SAVE);               
                btn_count = 0;
                jsonConfig["SYSTEM"]["NAME"]        = DEVICE_NAME;
                configSave();  
                waitMP3andReboot();
   
//                waitMP3(10000);    
//                ESP.restart();  
             }
             break;
          case SB_RELEASE:
             tm = btn.getPressTime();
             if( tm >= 2000 && tm < 10000 ){
                 is_early = false;
                 systemMP3("97",97,PRIORITY_MP3_HIGH);
                 startCalibrate(jsonConfig["CALIBR"]["DELAY_START"].as<uint32_t>()*1000);
                 if( bootCount>=1 ){
                    if( isAP )isAP = false;
                    else isAP = true; 
 //                   Serial.printf("!!! WiFi is %d\n",(int)isAP);
                 }
             }
             else {
                lastSensorOn = SS_RESTORE;
                is_early = true;
             }
//             else if( tm < 2000 && btn_cont )
             Serial.printf("!!! BTN Release %d\n",(int)tm);
             break;
          case SB_TIMER_COUNT:
             if( is_early){
               EventMP3->stop();             
               systemMP3("97",96,PRIORITY_MP3_HIGH);
             }
             is_early = false;
             break;   
          case SB_TIMER:
             Serial.printf("!!! BTN Timer %d\n",(int)btn.getPressTime());
             EventRGB1->set(COLOR_GROUND,COLOR_GROUND);               
             EventRGB2->set(COLOR_GROUND,COLOR_GROUND);               
             break;
          case SB_TIMER2:
             is_early = false;
             EventRGB1->set(COLOR_SAVE,COLOR_SAVE);               
             EventRGB2->set(COLOR_SAVE,COLOR_SAVE);  
             systemMP3("98",98,PRIORITY_MP3_HIGH);
//             jsonSave["BOOT_COUNT"]  = 0;
//             saveSave();
             jsonConfig["SYSTEM"]["PASS0"]       = DEVICE_PASS0;               //Пароль суперадминистратора
             jsonConfig["SYSTEM"]["PASS1"]       = DEVICE_PASS1;               //Пароль администратора
             jsonConfig["SYSTEM"]["PASSS"]       = DEVICE_PASSS;               //Пароль администратора
             configSave();
             waitMP3andReboot();

//             waitMP3(10000);    
//             ESP.restart();  
             break;
      }
       vTaskDelay(200);
       
   }

}




void setVolumeMP3(){
   EventMP3->setVolume(jsonConfig["MP3"]["VOLUME"].as<int>());
}


void baseMP3( JsonObject _config, bool is_delay ){
    bool _enable         = _config["ENABLE"].as<bool>();
    int _num             = _config["NUM"].as<int>();
    bool _loop           = _config["LOOP"].as<bool>();
    uint32_t _color      = _config["COLOR"].as<uint32_t>();
    uint32_t _ctimer     = _config["COLOR_TM"].as<uint32_t>()*1000;
    uint32_t _timer      = DEFAULT_TIMER_MP3 ;
    if( _loop )_timer    = 0;
    uint32_t _delay      = 0;
    if( is_delay )_delay = _config["DELAY"].as<uint32_t>()*1000;
    if(_enable){
//       EventMP3->stop();
       EventMP3->setColor( _color, _ctimer );
       EventMP3->start(MP3_BASE_DIR, _num, PRIORITY_MP3_MINIMAL, _delay, _timer, true, _loop);
    }
}
 
void systemMP3(char *_check, int _num, int _priority ){
    if( !jsonConfig["MP3"][_check]["ENABLE"].as<bool>() )return;
//    if( _busy && EventMP3->State != ES_NONE )return;
    EventMP3->setColor( COLOR_MP3_1 ,0);
    EventMP3->start(MP3_ADD_DIR, _num, _priority );
//    if( _wait )waitMP3(DEFAULT_TIMER_MP3);


}

void playMP3(int _dir, int _num, int _priority, uint32_t _color){
 //   EventMP3->stop();
    EventMP3->setColor( _color ,0);
    EventMP3->start(_dir, _num, _priority);
    
}

void waitMP3(uint32_t _delay){ 
    uint32_t _ms = millis() + _delay;
    Serial.println(F("!!! MP3 start wait ..."));
    while( EventMP3->State != ES_NONE ){
       if( _delay > 0 && _ms < millis() )break;
       vTaskDelay(500);   
    }
    Serial.println(F("!!! MP3 stop wait ..."));
}


void waitMP3andReboot(){
   EventRGB1->set(COLOR_SAVE,COLOR_SAVE,COLOR_BLACK,COLOR_BLACK,250,250);               
   EventRGB2->set(COLOR_SAVE,COLOR_SAVE,COLOR_BLACK,COLOR_BLACK,250,250);     
   calibrMode = CM_WAIT_REBOOT;          
}