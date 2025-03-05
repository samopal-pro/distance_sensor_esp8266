#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "WC_Proc.h"

#include <EEPROM.h>
#include "src/ESP8266Ping.h"        // https://github.com/dancol90/ESP8266Ping

#include "Config.h"
#include "WC_HTTP.h"
#include "WC_Led.h"
#include "Archive.h"

WiFiClient ws_client;

T_SENSOR_TYPE sensorType      = DEFAULT_SENSOR_TYPE;
//T_NAN_VALUE_FLAG nanValueFlag = DEFAULT_NAN_VALUE_FLAG;

long  t_correct      = 0;
uint32_t cur_ms, ms0=0, ms1=0, ms2=0, ms3=0, ms4=0, ms5=0, ms6=0, ms7=0;
uint32_t ms11   = 0, ms12 = 0;
unsigned long ms_err  = 0;
unsigned long t_cur   = 0;
unsigned long t_loop  = 0;
unsigned long t_http  = 0;
unsigned long t_stat2 = 0;
uint32_t LoopInterval = 1000;
uint32_t ms_http_last = 0;
uint32_t ms_stat_last = 0;
bool is_btn_click = false;

void setup() {
// Отключаем Serial на GPIO2  
   Serial1.end();
   Serial.begin(115200);
   delay(500);
//   for( int i=0; i<5;i++){delay(1000);Serial.print(".");}
   Serial.printf("\n\nFree memory %d\n",ESP.getFreeHeap());
   Serial.printf("Flash memoty size %d\n",ESP.getFlashChipSize());
   Serial.printf("Flash chip ID %d\n",ESP.getFlashChipId());
   Serial.println(_VERSION);
// Инициализация реле
   if( PinRelay >= 0 ){
       Serial.printf("Pin relay %d init\n",PinRelay);
       pinMode(PinRelay,OUTPUT);
       digitalWrite(PinRelay,LOW);   
   }
// Инициализация выхода на контроллер светофора
   if( PinController >= 0 ){
       Serial.printf("Pin controller %d init\n",PinController);
       pinMode(PinController,OUTPUT);
       digitalWrite(PinController,LOW);   
   }

// Проинициализировать WDT
//   WDT_init();
//   WDT_disable();

// Инициализация датчика температуры и влажности
   if( PinDHT22 < 0 ){
       Serial.printf("DHT22 is disabled\n");
   }
   else {
      Serial.printf("Init DHT22 on %d pin ...\n",PinDHT22);
      dht.begin();
   }
   
   
// Инициализация EEPROM
   EA_begin();
//   Serial.printf("EEPROM archive values: %d\n",EA_Count);

// Считываем поседнее значение из EEPROM
   if( EA_Count > 0 ){
      EA_read_last();
      LastButton   = EA_Save.Button;
      LastDistance = EA_Save.Distance;
//      LastTemp     = EA_Save.Temp;
//      LastHum      = EA_Save.Hum;
   }
// Инициализация кнопки калибровки
//   pinMode(PinCalibrateSonar,INPUT_PULLUP);
   calbtn.SetLongClick(10000);
//   calbtn.SetAutoClick(10000,1000);
// Инициализация светодиодной ленты
   ledInit();

// WiFi при старте отключен   
   WiFi_stop("WiFi is Off");
// Инициализация сонара   
   InitSonar();   
   WS_setDistance();
   Relay_setDistance1();
   Relay_setDistance2();
   ms4 = millis();   
//   while(true){
//      ledSetBaseMode(LED_BASE_FREE);
//      delay(3000);
//      ledSetBaseMode(LED_BASE_BUSY);
//      delay(3000);
//   }
   if( EA_Config.CountBoot < 1 ){ // 01.12.24 исправил с 2 на 1
       EA_Config.CountBoot++;
       EA_save_config();
       Serial.printf("!!! Calibrate sonar %d\n",(int)Distance);
       ProcessingCalibrate(0);
       isWiFiAlways1 = true;
//       WiFi_startAP();
   }
//   Serial.printf("!!! !!! isAP %d %d\n",(int)EA_Config.isWiFiAlways,(int)isWiFiAlways1);

   if( EA_Config.isWiFiAlways ||  isWiFiAlways1 )WiFi_startAP();
}


void loop() {
   cur_ms       = millis();
   t_cur        = cur_ms/1000;
// Действие по длинному нажатию кнопки
   if( ms0 == 0 || cur_ms < ms0 || (cur_ms - ms0) > 200  ){
       ms0 = cur_ms;
       switch(calbtn.Loop()){
          case SB_NONE:
              is_btn_click = false;
              break;     
          case SB_WAIT:
              is_btn_click = true;
              ledSetBaseMode(LED_BASE_NONE);
              break;     
          case SB_CLICK:
              Serial.printf("!!! BTN click %d\n",calbtn.Time);
              if( calbtn.Time > 1000 ){
//                 ledSetExtMode(LED_EXT_BTN3);
                 if( EA_Config.isWiFiAlways == false  && isWiFiAlways1 == false )
                    if( w_stat2 == EWS_AP_MODE )WiFi_stop("Stop AP User");
                    else WiFi_startAP();
                 Serial.printf("!!! Calibrate sonar %d\n",(int)Distance);
                 ProcessingCalibrate(EA_Config.TM_BEGIN_CALIBRATE*1000);

//                 EA_Config.GroundLevel = Distance;
                 EA_clear_arh();
//             EA_default_config();
//                EA_read_config();
 
              }
              break;
          case SB_LONG_CLICK:
              Serial.println("!!! BTN long click");
//              CalibrateGround();
              strcpy(EA_Config.ESP_NAME,DEVICE_NAME);
              strcpy(EA_Config.ESP_ADMIN_PASS, DEVICE_ADMIN);
              strcpy(EA_Config.ESP_OPER_PASS, DEVICE_OPER);
              EA_Config.CountBoot          = 0;
              EA_save_config();     
              delay(1000);    
              ESP.reset();  
   
              break;
       }
   }
// Цикл опроса всех сенсоров   

//   uint32_t it1 = LoopInterval;
//   if( w_stat2 == EWS_AP_MODE )it1 = LoopIntervalAP;
   if( ( ms1 == 0 || cur_ms < ms1 || (cur_ms - ms1) > LoopInterval )&& !is_btn_click && msLoad == 0 ){
      ms1 = cur_ms;
// Проверяем дистанцию, устанавливаем значение реле и ленты      
      ProcessingDistance();
      WS_setDistance();
      Relay_setDistance1();
      Relay_setDistance2();
//      if( RTCFlag ){
//          RTC_Time = GetRTClock();
//          Time     = RTC_Time;
//      }
      if( PinDHT22 < 0 ){
         Hum  = NAN;
         Temp = NAN;
      }
      else {
         Hum  = dht.readHumidity();
         Temp = dht.readTemperature();
      }
      
      EA_save_last(Time,cur_ms/1000, Button, Distance, SAVE_DISTANCE_DELTA);
      
   }
// Цикл проверки WiFi   
   if( ( cur_ms < ms2 || (cur_ms - ms2) > 5000 ) ){
      ms2 = cur_ms;
      if( msLoad != 0 && (millis()-msLoad > TM_HTTP_LOAD) )msLoad = 0;
#if defined(DEBUG1)         
      Serial.printf("!!! Mode = %d flag = %d #%06lX #%06lX\n",ledBaseMode,(int)EA_Config.isColorFreeBlink, EA_Config.ColorBlink,EA_Config.ColorFree);
#endif
      if( ledBaseMode == LED_BASE_FREE && EA_Config.isColorFreeBlink ){
         ledSetColor(EA_Config.ColorBlink);
         delay(100);
         ledSetColor(EA_Config.ColorFree);
      }
      WiFi_test();
   }
// Основной цикл обмена с сервером   
   if( ( ms3 == 0 || cur_ms < ms3 || (cur_ms - ms3) > 1000 )&& ( w_stat2 != EWS_AP_MODE )){
      ms3 = cur_ms;     

//      Serial.printf("!!! !!! %d %d\n",(int)EA_Config.isSendCrmMoscow,(int)w_stat2);
      if( EA_Config.isSendCrmMoscow &&  w_stat2 == EWS_ON ){     
         bool ret = false, ping_flag = false;
         if( ms_stat_last ==0 || ms_stat_last < cur_ms ){
#if defined(PING_SERVER)
            ping_flag = Ping.ping(EA_Config.SERVER);
#else 
            ping_flag = true;        
#endif
            if( ping_flag )HttpGetStatus();
            ms_stat_last = cur_ms + GetStatusInterval;
        }
// Если поменялось состояние отправляем данные            
        if( isChangeStat ){
#if defined(PING_SERVER)
            ping_flag = Ping.ping(EA_Config.SERVER);
#else 
            ping_flag = true;        
#endif
            if( ping_flag )ret = HttpSetParam(Time, cur_ms/1000, Temp, Hum, Distance, Button, 0);
            else ret = false;
            isChangeStat = false;
            if( ret )ms_http_last = cur_ms + EA_Config.TM_HTTP_SEND*1000;
            else     ms_http_last = cur_ms + EA_Config.TM_HTTP_RETRY_ERROR*1000;               
        }
// Если не поменялось, смотрим наступило ли время очередной отправки   
        else if( ms_http_last == 0 || ms_http_last < cur_ms ){
#if defined(PING_SERVER)
            ping_flag = Ping.ping(EA_Config.SERVER);
#else 
            ping_flag = true;        
#endif
            if( ping_flag )ret = HttpSetParam(Time, cur_ms/1000, Temp, Hum, Distance, Button, 0);
            else ret = false;
            isChangeStat = false;
            if( ret )ms_http_last = cur_ms + EA_Config.TM_HTTP_SEND*1000;
            else     ms_http_last = cur_ms + EA_Config.TM_HTTP_RETRY_ERROR*1000;               
        }   

/*            
// Если есть архив, посылаем сперва архив           
           if(  EA_Count>0  ){
              HttpArchiveParam();
           }
           else {
// Если есть значения в стеке памяти                       
              if( EA_Ram.count() > 0 ){
                 struct EA_SaveType Val = EA_Ram.pop();
// Если не удалось записать значение, то помещаем обратно в стек памяти                 
                 if( !HttpSetParam(Val.Time,Val.Uptime,Val.Temp,Val.Hum,Val.Distance,Val.Button,Val.Flag) )EA_Ram.push(Val);
                 else ms4 = cur_ms;
                    
              }
           }
 */
      }
/*      
// Если нету связи, переписываем все значения из памяти в архив      
      while( flag == false && EA_Ram.count() > 0 ){
         struct EA_SaveType Val = EA_Ram.pop();
         int c_interval = ButtonArhIntervalOff;
         if( Val.Button )c_interval = ButtonArhIntervalOn; 
#if !defined(WIFI_SAV)         
         EA_save(Val.Time,Val.Temp,Val.Hum,Val.Distance,Val.Button,c_interval);
// Последнее значение сохраняем для контроля при перезагрузки         
         if( EA_Ram.count() == 0 ){
            EA_save_last(Val.Time,Val.Uptime,Val.Temp,Val.Hum,Val.Distance,Val.Button,0);
         }
#endif         
       }
       */
      
   }
/*   
// Отправляем состояние по таймеру
   if( ( ms4 == 0 || cur_ms < ms4 || (cur_ms - ms4) > SendInterval )){
      ms4 = cur_ms;
      if( w_stat2 == EWS_ON ){
//        if( Ping.ping(EA_Config.SERVER) ){
//           HttpSetStatus(Time,millis()/1000,Temp,Hum,Distance);
           HttpSetParam(Time,millis()/1000,Temp,Hum,Distance,Button,1); 
//        }
      }
   }
*/  
//   if( ms5 == 0 || cur_ms < ms5 || (cur_ms - ms5) > LED_TM_DEFAULT ){
//     ms5 = cur_ms;
//      /*if( w_stat2 != EWS_AP_MODE )*/ledLoop();
//   }  
//   if( ms6 == 0 || cur_ms < ms6 || (cur_ms - ms6) > 10 ){
//      ms6 = cur_ms;
      if( w_stat2 == EWS_AP_MODE )HTTP_loop();
//   }  
   
}


