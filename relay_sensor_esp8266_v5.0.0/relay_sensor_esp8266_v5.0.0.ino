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
unsigned long ms_err = 0;
unsigned long t_cur  = 0;
unsigned long t_loop = 0;
unsigned long t_http = 0;
unsigned long t_stat2 = 0;

bool is_btn_click = false;

void setup() {
// Отключаем Serial на GPIO2  
   Serial1.end();
   Serial.begin(115200);
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
   WDT_init();
   WDT_disable();

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
   Serial.printf("EEPROM archive values: %d\n",EA_Count);
// Считываем поседнее значение из EEPROM
   if( EA_Count > 0 ){
      EA_read_last();
      LastButton   = EA_Save.Button;
      LastDistance = EA_Save.Distance;
      LastTemp     = EA_Save.Temp;
      LastHum      = EA_Save.Hum;
   }
// Инициализация кнопки калибровки
//   pinMode(PinCalibrateSonar,INPUT_PULLUP);
   calbtn.SetLongClick(10000);
//   calbtn.SetAutoClick(10000,1000);
// Инициализация светодиодной ленты
   ledInit();

// WiFi при старте отключен   
   WiFi_stop("WiFi is Off");
   if( EA_Config.isWiFiAlways)WiFi_startAP();
// Инициализация сонара   
   InitSonar();   
   WS_setDistance();
   Relay_setDistance();
   ms4 = millis();   
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
              Serial.printf("!!! BTN click %d",calbtn.Time);
              if( calbtn.Time > 1000 ){
//                 ledSetExtMode(LED_EXT_BTN3);
                 if( EA_Config.isWiFiAlways == false )
                    if( w_stat2 == EWS_AP_MODE )WiFi_stop("Stop AP User");
                    else WiFi_startAP();
                 Serial.printf("Calibrate sonar %d\n",Distance);
                 if( CalibrateGround() )EA_save_config();

//                 EA_Config.GroundLevel = Distance;
                 EA_clear_arh();
//             EA_default_config();
//                EA_read_config();
 
              }
              break;
          case SB_LONG_CLICK:
              Serial.println("!!! BTN long click");
              CalibrateGround();
              strcpy(EA_Config.ESP_NAME,DEVICE_NAME);
              strcpy(EA_Config.ESP_ADMIN_PASS, DEVICE_ADMIN);
              EA_save_config();            
              break;
       }
   }
// Цикл опроса всех сенсоров   

//   uint32_t it1 = LoopInterval;
//   if( w_stat2 == EWS_AP_MODE )it1 = LoopIntervalAP;
   if( ( ms1 == 0 || cur_ms < ms1 || (cur_ms - ms1) > LoopInterval )&& !is_btn_click){
      ms1 = cur_ms;
// Проверяем дистанцию, устанавливаем значение реле и ленты      
      ProcessingDistance();
      WS_setDistance();
      if( RTCFlag ){
          RTC_Time = GetRTClock();
          Time     = RTC_Time;
      }
      if( PinDHT22 < 0 ){
         Hum  = NAN;
         Temp = NAN;
      }
      else {
         Hum  = dht.readHumidity();
         Temp = dht.readTemperature();
      }
      
   }
// Цикл проверки WiFi   
   if( ( cur_ms < ms2 || (cur_ms - ms2) > 5000 ) ){
      ms2 = cur_ms;
      WiFi_test();
   }
// Основной цикл обмена с сервером   
   if( ( ms3 == 0 || cur_ms < ms3 || (cur_ms - ms3) > GetStatusInterval )){
      ms3 = cur_ms;
      if(w_stat2 != EWS_AP_MODE){
         PrintValue();
      }
      
      
      bool flag = false;
      if( w_stat2 == EWS_ON ){
        flag = Ping.ping(EA_Config.SERVER);
//        flag = true;
        if( flag ){
// Запрос состояния
           HttpGetStatus();
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
        }  
        else {
           Serial.printf("!!! No ping %s\n",EA_Config.SERVER);
        }    
      }
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
   }
// Отправляем состояние по таймеру
   if( ( ms4 == 0 || cur_ms < ms4 || (cur_ms - ms4) > SendInterval )){
      ms4 = cur_ms;
      if( w_stat2 == EWS_ON ){
        if( Ping.ping(EA_Config.SERVER) ){
           HttpSetStatus(Time,millis()/1000,Temp,Hum,Distance);
        }
      }
   }
//   if( ms5 == 0 || cur_ms < ms5 || (cur_ms - ms5) > LED_TM_DEFAULT ){
//     ms5 = cur_ms;
//      /*if( w_stat2 != EWS_AP_MODE )*/ledLoop();
//   }  
//   if( ms6 == 0 || cur_ms < ms6 || (cur_ms - ms6) > 10 ){
//      ms6 = cur_ms;
      if( w_stat2 == EWS_AP_MODE )HTTP_loop();
//   }  
   
}


