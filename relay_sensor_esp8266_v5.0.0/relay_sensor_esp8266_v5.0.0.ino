#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "WC_Proc.h"

#include <Ticker.h>

#include <EEPROM.h>
#include "src/ESP8266Ping.h"        // https://github.com/dancol90/ESP8266Ping

#include "Config.h"
#include "WC_HTTP.h"
#include "Archive.h"

WiFiClient ws_client;
Ticker timerDistance; 




long  t_correct      = 0;
unsigned long cur_ms = 0;
unsigned long ms1    = 0;
unsigned long ms2    = 0;
unsigned long ms3    = 0;
unsigned long ms4    = 0;
unsigned long ms5    = 0;
unsigned long ms6    = 0;
unsigned long ms7    = 0;
unsigned long ms11   = 0, ms12 = 0;
unsigned long ms_err = 0;
unsigned long t_cur  = 0;
unsigned long t_loop = 0;
unsigned long t_http = 0;
unsigned long t_stat2 = 0;

Ticker _tickerDistance;
ICACHE_RAM_ATTR void _getDistance();

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
   
// Инициализация часов РВ
   if( RTCFlag ){
       Wire.begin(PinRTC_SDA,PinRTC_SCL);
       if (! rtc.begin()) {
          Serial.print(F("DS3231 not found\n"));
          RTCFlag = false;
      }
   }
// Устанавливаем время в Time   
   if( RTCFlag ){      
      Serial.printf("Init RTC DS3231 sda=%d, sck=%d ...\n",PinRTC_SDA,PinRTC_SCL);
      RTC_Time = GetRTClock();
      Time = RTC_Time;
      PrintTime(Time);
      is_first_flag = false;
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
   calbtn.SetLongClick(3000);

// Инициализация светодиодной ленты
   if( WS_PIN >= 0 ){
      strip.begin();
      WS_set(1);
   }
// WiFi при старте отключен   
   WiFi_stop("WiFi is Off");
// Инициализация сонара   
   InitSonar();   
   WS_setDistance();
   Relay_setDistance();
//   _tickerDistance.attach(1,_getDistance);
   ms4 = millis();   
}

void _getDistance(){
  _tickerDistance.detach();
  ProcessingDistance();
   _tickerDistance.attach(1,_getDistance);
}


void loop() {
   cur_ms       = millis();
   t_cur        = cur_ms/1000;
// Действие по длинному нажатию кнопки
   if( PinCalibrateSonar >= 0 ){
       if( calbtn.Loop() == SB_LONG_CLICK ){
          if( w_stat2 == EWS_AP_MODE ){
             WiFi_stop("Stop AP User");     
          }
          else {
             Serial.printf("Calibrate sonar %d\n",Distance);
             EA_default_config();
             EA_Config.GroundLevel = Distance;
             EA_clear_arh();
             EA_save_config();
             EA_read_config();
             WiFi_startAP();
          }
       }
   }
// Цикл опроса всех сенсоров   
   if( ( cur_ms < ms1 || (cur_ms - ms1) > LoopInterval )/* &&( WiFi.status() == WL_CONNECTED ) */ ){
      ms1 = cur_ms;
// Проверяем дистанцию, устанавливаем значение реле и ленты      
      ProcessingDistance();
      if( RTCFlag ){
          RTC_Time = GetRTClock();
          Time     = RTC_Time;
      }
      if( PinDHT22 < 0 ){
         Hum  = 0;
         Temp = 0;
      }
      else {
         Hum  = (int)dht.readHumidity();
         Temp = (int)dht.readTemperature();
      }
      
   }
// Цикл проверки WiFi   
   if( ( cur_ms < ms2 || (cur_ms - ms2) > 5000 )){
      ms2 = cur_ms;
      WiFi_test();
   }
// Основной цикл обмена с сервером   
   if( ( ms3 == 0 || cur_ms < ms3 || (cur_ms - ms3) > GetStatusInterval )){
      ms3 = cur_ms;
         Serial.print("Time=");    
         Serial.print(Time);
         Serial.print(",T(C)=");    
         Serial.print(Temp);
         Serial.print(",H(%)=");
         Serial.print(Hum);
         Serial.print(",D(mm)=");
         Serial.print(Distance);
         Serial.print(",Bt=");
         Serial.print(Button);
         Serial.print(",LastBt=");
         Serial.print(LastButton);
         Serial.print(",GetStatus Time = ");
         Serial.println(millis()-cur_ms); 

      
      
      bool flag = false;
      if( w_stat2 == EWS_ON ){
        flag = Ping.ping(EA_Config.SERVER);
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
      }
// Если нету связи, переписываем все значения из памяти в архив      
      while( flag == false && EA_Ram.count() > 0 ){
         struct EA_SaveType Val = EA_Ram.pop();
         int c_interval = ButtonArhIntervalOff;
         if( Val.Button )c_interval = ButtonArhIntervalOn; 
         EA_save(Val.Time,Val.Temp,Val.Hum,Val.Distance,Val.Button,c_interval);
// Последнее значение сохраняем для контроля при перезагрузки         
         if( EA_Ram.count() == 0 ){
            EA_save_last(Val.Time,Val.Uptime,Val.Temp,Val.Hum,Val.Distance,Val.Button,0);
         }
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
   if( WiFi.getMode() == WIFI_AP )HTTP_loop();
}
