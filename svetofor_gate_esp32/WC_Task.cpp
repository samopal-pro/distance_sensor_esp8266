#include "WC_Task.h"
char strID[16];
uint64_t chipID;
char serNo[33];

int MP3_ADD_DIR = MP3_SYSTEM_FULL_DIR;

TEventRGB *EventRGB1;
//TEventRGB  *SaveEventRGB2, *SaveEventCalibrate1, *SaveEventCalibrate2;
//TSaveRGB *SaveRGB1, *SaveRGB2;


TEventMP3 *EventMP3 = NULL;

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
SemaphoreHandle_t buttonSemaphore, SemaphoreIR;
uint32_t colorMP3   = COLOR_SAVE;

bool isSendNet = false; // Флаг отсылки парамтров на сервер



void readID(){
    union uint32_to_uint8 {
       uint32_t data32;
       char  data8[4];
    };
   uint8_t mac[6];
  
   esp_efuse_mac_get_default(mac);
   sprintf(strID, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
   chipID = 0;
   for (int i = 0; i < 6; i++) {
      chipID <<= 8;
      chipID |= mac[i];
  }
  uint32_to_uint8 dd;
  for( int i=0; i<8; i++){
     dd.data32 = esp_efuse_read_reg(EFUSE_BLK3, 7-i);
     for( int j=0; j<4; j++)serNo[i*4+j] = dd.data8[3-j];
  }
  serNo[32] = '\0';
  Serial.printf("!!! ESP ID=%012llX strID=%s serNO=%s\n",chipID,strID,serNo);
}


/**
 * Старт всех параллельных задач
 */
void tasksStart() {
  Serial.print(F("!!! Free mem: "));
  Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

  readID();
  configInit();
//  configDefault();
//  configSave();
//  configDefault();
  configRead();
  nodeListRead();
  
  sensorSemaphore = xSemaphoreCreateMutex();
  soundSemaphore  = xSemaphoreCreateMutex();
  buttonSemaphore = xSemaphoreCreateBinary();
  SemaphoreIR     = xSemaphoreCreateMutex();


   EventRGB1           = new TEventRGB(PIN_RGB1, COUNT_RGB1, 2);
//   EventRGB2           = new TEventRGB(PIN_RGB2, COUNT_RGB2, 0);
   EventRGB1->setBrightness( jsonConfig["RGB1"]["BRIGHTNESS"].as<int>() );
//   EventRGB2->setBrightness( jsonConfig["RGB2"]["BRIGHTNESS"].as<int>() );

   isPlayMP3 = true;
   FPSerial.begin(9600, SERIAL_8N1, PIN_RX1, PIN_TX1);
   EventMP3 = new TEventMP3(Serial1,handleMP3);
//   EventMP3->setVolume(jsonConfig["MP3"]["VOLUME"].as<int>());

 //  xTaskCreateUniversal(taskRGB, "rgb", 2048, NULL, 3, NULL, CORE);
 //  xTaskCreateUniversal(taskMP3, "mp3", 2048, NULL, 1, NULL, CORE);
 
   xTaskCreateUniversal(taskButton, "btn", 2048, NULL, 4, NULL,CORE);
   vTaskDelay(500);
   xTaskCreateUniversal(taskNet, "net", 4096, NULL, 3, NULL, CORE);
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
      if( t != ERT_RAINBOW )vTaskDelay(250);
      else vTaskDelay(25);
//      if( ms1 == 0 || ms1 > _ms || _ms-ms1 > 1000){
//          ms1 = _ms;
//      Serial.printf("!!! Rgb status %d %d\n", t, (int)EventRGB1->isRainbow);
//      }

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
             break;
          case SB_RELEASE:
             break;
          case SB_TIMER_COUNT:
             break;   
          case SB_TIMER:
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
}