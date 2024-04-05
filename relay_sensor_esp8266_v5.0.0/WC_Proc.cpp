/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#include "WC_PROC.h"
#include "Config.h"
#include "Archive.h"

float Temp = NAN, LastTemp = NAN, Hum = NAN, LastHum = NAN;;
float LastDistance = NAN, Distance = NAN;

SonarFake FSonar(DEPTH_DIST_ARRAY);

bool  Button       = false;
bool  LastButton   = false;
int   ButtonCountOn  = 0;
int   ButtonCountOff = 0;
bool   PinCalibrateSonarState = false;
char sbuf[SendBuferSize];
char rbuf[ReceiveBuferSize];

unsigned long Time   = 0;
unsigned long RTC_Time   = 0;
unsigned int DO      = 0;
bool   WDT_active = false;
int     FlagStat     = 0;
unsigned int ErrCount = 0;
unsigned int TryCount = 0;
bool     ResetByErr   = false;
const char KeyGenStrParam[]      = "%s;%ld;%d;%d;%d";
char SensorID[32];

unsigned long ButtonTime  = 0;


ES_STAT stat2           = STAT_OFF;
bool is_first_flag   = true;
unsigned long t_ws_send = 0;
unsigned long t_ws_recv = 0;


bool RTCFlag  =  true;
bool FlagWDT  =  false;
RTC_DS3231 rtc;
DHT dht(PinDHT22, DHT22);
SoftwareSerial ultraSensor(PinDistanceEcho,-1);
SoftwareSerial lidarTF(PinDistanceTrig,PinDistanceEcho);
SButton calbtn( PinCalibrateSonar );
TFMPI2C tfmP;   
TFLI2C tflI2C;

float L = NAN, H = NAN;
bool isSonarEnable = false;

/**
 * Инициализация ультразвукового датчика
 */
void InitSonar(){
// Иницализация ультразвукового датчика
   isSonarEnable = false;
   switch(sensorType){
       case SONAR_SR04T :
       case SONAR_SR04TV2 :
       case SONAR_SR04TM2 :
          if( PinDistanceTrig >= 0 ){
             Serial.printf("Init Sonar trig=%d, echo=%d ...\n",PinDistanceTrig,PinDistanceEcho);
             pinMode(PinDistanceTrig , OUTPUT);
             pinMode(PinDistanceEcho , INPUT);
             digitalWrite(PinDistanceEcho , HIGH);  
             isSonarEnable = true;
          }       
          break;
       case SONAR_SERIAL :
          if( PinDistanceEcho >= 0 ){
             Serial.printf("Init Sonar serial type RX=%d ...\n",  PinDistanceEcho);
             isSonarEnable = true;
          }
          break;
       case SONAR_TFMINI :
          Wire.begin(PinDistanceEcho,PinDistanceTrig);
          if( scanI2C(0x10) == false ){
              Serial.println("Check UART TFMINI...");
              lidarSetI2C();
              Wire.begin(PinDistanceEcho,PinDistanceTrig);
              scanI2C(0x10);
          }
          if( tfmP.sendCommand( SOFT_RESET, 0) ){
              Serial.println("Init LiDAR TFMini");
              isSonarEnable = true;
          }
          else {
              Serial.println("Error LiDAR TFMini");
              tfmP.printReply();
          }
          break;
       case SONAR_TFLUNA :
          Wire.begin(PinDistanceEcho,PinDistanceTrig);
          scanI2C(0x10);
          if( tflI2C.Soft_Reset(0x10) ){
              Serial.println("Init LiDAR TFLuna");
              isSonarEnable = true;
          }
          else {
              Serial.println("Error LiDAR TFLuna");
              tflI2C.printStatus();
          }   
          break; 
    }
    if( !isSonarEnable )Serial.println("Sonar is disabled");

    if( PinCalibrateSonar >= 0 ){
       pinMode(PinCalibrateSonar , INPUT);
       delay(1000);
       PinCalibrateSonarState = digitalRead(PinCalibrateSonar);
       Serial.printf("Pin calibrate sonar %d normal state %d\n",PinCalibrateSonar,PinCalibrateSonarState);
    }  
}  

/**
* Чтение расстояния с ультразвукового датчика
*/
void GetDistance(){
   if(!isSonarEnable)return;
   switch(sensorType){
       case SONAR_SR04T :
          GetDistanceSR04(2, 10, 4, 0, 7000);
          break;        
       case SONAR_SR04TV2 :
          GetDistanceSR04(2, 10, 1);
          break;        
       case SONAR_SR04TM2 :
          GetDistanceSR04(2, 500, 4, 0, 5000);
          break;        
       case SONAR_SERIAL :
          GetDistanceSerial();
          break;
       case SONAR_TFMINI :
          GetDistanceTFMini();
          break;
       case SONAR_TFLUNA :
          GetDistanceTFLuna();
          break;
   }
}



void GetDistanceSR04(uint32_t tm1, uint32_t tm2, int samples,float min, float max){
   L = NAN;
   float distAvg = 0, distMin = 99999, distMax = 0;
   float distArray[samples];
   int n = 0;
   for( int i=0; i<samples; i++){
      digitalWrite(PinDistanceTrig, LOW);
      delayMicroseconds(tm1);
      digitalWrite(PinDistanceTrig, HIGH);
      delayMicroseconds(tm2);
      digitalWrite(PinDistanceTrig, LOW);
      uint32_t _dur = pulseIn(PinDistanceEcho, HIGH);
      float   _dist = _dur/5.8;
      if( !isnan(min) )if(_dist < min)continue;
      if( !isnan(max) )if(_dist > max)continue;
      n++;
      distArray[i]  = _dist;
      distAvg += _dist;
      if(  distMin > _dist)distMin = _dist;
      if(  distMax < _dist)distMax = _dist;
      delay(50);
   }
// Вычислем среднеквадратичное отклонение   
   if( n == 0 )return;
   distAvg /= samples;
   float distDiv = 0;
   for( int i=0; i<samples; i++){
      distDiv += (distArray[i]-distAvg)*(distArray[i]-distAvg);
   }
   distDiv /= samples;
   distDiv = sqrt(distDiv);
// Проверяем на достоверность 
   if( distDiv/distAvg < RELIABILITY_PROC )L = distAvg;
}

void GetDistanceTFMini(){
   int16_t _dist, _flux, _temp;
   tfmP.getData( _dist, _flux, _temp);
   if( tfmP.status == TFMP_READY){
      Serial.printf("TFMini dist=%d flux=%d temp=%d\n",(int)_dist, (int)_flux, (int)_temp);
      L = (float)_dist*10.0;    
   }
   else {
      Serial.printf("Error LiDAR dist\n");
      L = NAN;    
   }
}

void GetDistanceTFLuna(){
   int16_t _dist, _flux, _temp;
   if( tflI2C.getData( _dist, _flux, _temp, 0x10) ){
      Serial.printf("TFLuna dist=%d flux=%d temp=%d\n",(int)_dist, (int)_flux, (int)_temp);
      L = (float)_dist*10.0;    
   }
   else {
      Serial.printf("Error LiDAR dist\n");
      L = NAN;    
   }
}

/**
 * Опрос ультразвукового датчика с мигающим светодиодом
 */
void GetDistanceSerial(){ 
   byte readByte;
   byte crcCalc;
   word distance = 0;
   L = NAN;

  //
  // Проверка наличия данных в COM порту
  //
  uint32_t ms_start = millis();
  ultraSensor.begin(9600);
  while( true ){
     if( ultraSensor.available() > 3 )break;
     uint32_t _ms = millis();
     if( ms_start > _ms || _ms - ms_start > 1000 )break;
  }
  if( ultraSensor.available() > 3 ){
     byte b0   = ultraSensor.read();
     byte b1   = ultraSensor.read();
     byte b2   = ultraSensor.read();
     byte b3   = ultraSensor.read();
     crcCalc   = b0 + b1 + b2;     
     if( crcCalc == b3 ){
        distance = (b1 * 0xff) + b2;
        L = (float)distance;
     }
  }
  ultraSensor.flush();
  ultraSensor.end();
}




/**
 * Формируем запрос состояния с сервера
 */
bool HttpGetStatus(void){
   WiFiClient client;
//   IPAddress ip1;
//   WiFi.hostByName(EA_Config.SERVER, ip1);
//   Serial.print("IP=");
//   Serial.println(ip1);
//   strcpy(EA_Config.SERVER,"51.158.190.232");
// Подключаемся к WEB ерверу
   Serial.printf("HTTP Stat: %s:%d ",EA_Config.SERVER,EA_Config.PORT);
   if (!client.connect(EA_Config.SERVER, EA_Config.PORT)) {
       Serial.println(F(" failed"));
       return false;
   }
// Формируем строку запроса  
   sprintf(sbuf,"GET http://%s:%d%sget_stat/?id=%s HTTP/1.0\r\n\r\n",EA_Config.SERVER,EA_Config.PORT,HTTP_PATH,SensorID);
// Посылаем строку на сервер
//   Serial.println(sbuf);
   ESP.wdtFeed();
   client.print(sbuf);
   delay(100);
   unsigned long start = millis();
   int i = 0;
   
   while (millis()-start<300 /*&& wifi.available()>0*/) {
       ESP.wdtFeed();
       if(client.available()>0)  {
           start = millis();
           char a  = client.read();
           if( i<ReceiveBuferSize-1 )rbuf[i++] = a;
       }    
   }
   rbuf[i] = '\0';
// Парсим ответ

/// Находим время
   char *p1 = strstr(rbuf,"TIME=");
   if( p1 == NULL ){
       Serial.print(F("Can't requset TIME and DO.\n"));
       return false;
   }
   p1 += 5;
   char *p2 = strstr(p1,"DO=");
   char *p3 = strstr(p1,"WDT=");
   for( char *p = p1; *p != '\0'; p++ ){
       if( *p == ' ' || *p == '\r' || *p == '\n' ){
          *p = '\0';   
          break;
       }
   }
   Time  = atol(p1);

   if( CheckTime(Time) && abs( (long)(Time - GetRTClock()) )>5 ){
       Serial.print("Set RTC: ");
       PrintTime(Time);
       SetRTClock(Time);
   }
// Парсим DO
   if( p2 != NULL ){
      p2 += 3;
      for( char *p = p2; *p != '\0'; p++ ){
          if( *p == ' ' || *p == '\r' || *p == '\n' ){
             *p = '\0';   
             break;
          }
      }
      DO = atoi(p2);
   } 
// Парсим WDT
   if( p3 != NULL ){
      p3 += 4;
      for( char *p = p3; *p != '\0'; p++ ){
          if( *p == ' ' || *p == '\r' || *p == '\n' ){
             *p = '\0';   
             break;
          }
      }
      FlagWDT = (bool)atoi(p3);
   } 
   Serial.printf(" DO=0x%x WDT=%d Time=",DO,FlagWDT); 
   PrintTime(Time);   
     
   return true;        

}





bool HttpSetParam(uint32_t _time, uint32_t _uptime, int _temp, int _hum, int _dist, bool _btn, int _stat ){
   WiFiClient client;
// Подключаемся к WEB ерверу
   Serial.printf("HTTP Send: %s:%d ",EA_Config.SERVER,EA_Config.PORT);
   if (!client.connect(EA_Config.SERVER, EA_Config.PORT)) {
       Serial.println(F(" Connection failed"));
       return false;
   }
// Формируем строку запроса  

   sprintf(sbuf,"GET http://%s:%d%s?id=%s&temp=%d&hum=%d&dist=%d&tm=%ld&btn=%d&uptime=%ld&key=%d HTTP/1.0\r\n\r\n",
      EA_Config.SERVER,EA_Config.PORT,HTTP_PATH,SensorID,_temp,_hum,
      _dist,_time,(int)_btn,_uptime,(int)KeyGen());
// Посылаем строку на сервер
   Serial.print(sbuf);
   delay(100);
   ESP.wdtFeed();
   client.print(sbuf);
   delay(500);
   unsigned long start = millis();
   int i = 0;
   
   while (millis()-start<2000 ) {
       ESP.wdtFeed();
       if(client.available()>0)  {
           start = millis();
           char a  = client.read();
           if( i<ReceiveBuferSize-1 ){
               rbuf[i++] = a;
               rbuf[i]   = '\0';
           }
       }    
    }
//   rbuf[i] = '\0';
// Проверка ответа сервера
   if( CheckRequest ){
//       Serial.printf("Server request ");
       const char *p  = rbuf;
       if( strstr(p,"OK") == NULL ){
           Serial.println(" Request false");
           return false;     
            
           
       }
       Serial.println(" OK");
   }

   if( CheckRequest ){
       const char *p  = rbuf;
//       Serial.printf("Server request %s ",p);
   }

   
   return true;

  
}

bool HttpSetStatus(uint32_t _time, uint32_t _uptime, int _temp, int _hum, int _dist ){
   WiFiClient client;
// Подключаемся к WEB ерверу
   Serial.printf("HTTP Send: %s:%d ",EA_Config.SERVER,EA_Config.PORT);
   if (!client.connect(EA_Config.SERVER, EA_Config.PORT)) {
       Serial.println(F(" Connection failed"));
       return false;
   }
// Формируем строку запроса  

   sprintf(sbuf,"GET http://%s:%d%s?id=%s&temp=%d&hum=%d&dist=%d&tm=%ld&uptime=%ld&key=%d HTTP/1.0\r\n\r\n",
      EA_Config.SERVER,EA_Config.PORT,HTTP_PATH,SensorID,_temp,_hum,
      _dist,_time,_uptime,(int)KeyGen());
// Посылаем строку на сервер
   Serial.print(sbuf);
   delay(100);
   ESP.wdtFeed();
   client.print(sbuf);
   delay(500);
   unsigned long start = millis();
   int i = 0;
   
   while (millis()-start<2000 ) {
       ESP.wdtFeed();
       if(client.available()>0)  {
           start = millis();
           char a  = client.read();
           if( i<ReceiveBuferSize-1 ){
               rbuf[i++] = a;
               rbuf[i]   = '\0';
           }
       }    
    }
//   rbuf[i] = '\0';
// Проверка ответа сервера
   if( CheckRequest ){
//       Serial.printf("Server request ");
       const char *p  = rbuf;
       if( strstr(p,"OK") == NULL ){
           Serial.println(" Request false");
           return false;     
            
           
       }
       Serial.println(" OK");
   }

   if( CheckRequest ){
       const char *p  = rbuf;
//       Serial.printf("Server request %s ",p);
   }

   
   return true;

  
}


/**
 * Генерация контрольной суммы 
 */
uint16_t KeyGen(){
   sprintf_P(rbuf,KeyGenStrParam,SensorID,Time,Distance,Time,Hum);
   uint16_t crc = 0;
   for( int i=0; i< strlen(rbuf); i++ ){
       crc += (int)rbuf[i];
   }    
   crc = ( ~ crc )&0xfff;   
   return crc;
  
}


bool HttpArchiveParam(){
  if( WiFi.status() != WL_CONNECTED )return false;
  Serial.printf("->> EEPROM archive values: %d\n",EA_Count);
   if( EA_Count == 0 )return true;
// Считываем значения из архива   
   int cnt = EA_read10();
// Формируем строку запроса
   char *p = sbuf;
//   sprintf(sbuf,"GET %s?",HttpUrlParam);
   sprintf(sbuf,"GET http://%s:%d%s?id=%s&arh0=%ld,%d,%d,%d,%d",EA_Config.SERVER,EA_Config.PORT,HTTP_PATH,SensorID,
      EA_Buffer[0].Time,EA_Buffer[0].Temp,EA_Buffer[0].Hum,EA_Buffer[0].Distance,(int)EA_Buffer[0].Button);
//   strcat(sbuf,rbuf);

   for( int i=1; i<cnt; i++ ){
      sprintf(rbuf,"&arh%d=%ld,%d,%d,%d,%d",
         i,EA_Buffer[i].Time,EA_Buffer[i].Temp,EA_Buffer[i].Hum,EA_Buffer[i].Distance,(int)EA_Buffer[i].Button);
      strcat(sbuf,rbuf);
   }
   strcat(sbuf," HTTP/1.0\r\n\r\n");
   WiFiClient client;
// Подключаемся к WEB ерверу
   Serial.printf("HTTP Arh Send: %s:%d ",EA_Config.SERVER,EA_Config.PORT);
    if (!client.connect(EA_Config.SERVER, EA_Config.PORT)) {
       Serial.println(F(" Connection failed"));
       return false;
   }
// Посылаем строку на сервер
   Serial.println(sbuf);
//   delay(100);
   client.print(sbuf);
   delay(100);
   unsigned long start = millis();
   int i = 0;
   
   while (millis()-start<2000 ) {
       if(client.available()>0)  {
           start = millis();
           char a  = client.read();
           if( i<ReceiveBuferSize-1 ){
               rbuf[i++] = a;           
               rbuf[i] = '\0';
           }
       }    
   }
//   rbuf[i] = '\0';
   if( CheckRequest ){
       const char *p  = rbuf;
//       Serial.printf("Server request %s ",p);
       if( strstr(p,"OK") == NULL ){
           Serial.println(" Request false");
           return false;     
            
           
       }
       Serial.println(" OK");
   }
   EA_shift(cnt);

   return true;

  
}

/**
 * Функция инициализации WDT
 */
void WDT_init(){
   if( PinWDT >= 0 ){ 
       pinMode(PinWDT , OUTPUT);
       Serial.printf("WDT init on %d pin ...\n",PinWDT);
   }  
}


/**
 * Функция выключения WDT
 */
void WDT_disable(){
   if( PinWDT >= 0 ){
       digitalWrite(PinWDT,LOW);
       Serial.printf("WDT disable ...\n");
   }   
}


/**
 * Функция перезагрузки сторожевого
 */
void WDT_reset(){
   if( PinWDT >= 0 ){
       if( FlagWDT ){
          digitalWrite(PinWDT,LOW);
          delay(300);
          digitalWrite(PinWDT,HIGH);
          Serial.printf("WDT reset ...\n");
          WDT_active = false;
       }
       else {
          WDT_disable();   
       }
   }   
}


/**
 * Работа с временем
 */
unsigned long GetRTClock(){
  if( RTCFlag == false )return 0;
  DateTime dt = rtc.now();
  return dt.unixtime(); 
}


void SetRTClock(unsigned long Time){
  if( RTCFlag == false )return;
  rtc.adjust(DateTime(Time));
}

bool CheckTime(time_t t){
   bool ret = false;
   if( t > 1000000000UL && t < 2000000000UL )ret = true;
//   Serial.printf("%ld %ld %ld %d\n",t,1000000000UL,2000000000UL,(int)ret);
   return ret;
}

void PrintTime( time_t t ){
   DateTime dt = DateTime(t);
   Serial.printf("%02d.%02d.%04d %02d:%02d:%02d\n",dt.day(),dt.month(),dt.year(),dt.hour(),dt.minute(),dt.second());
}

/*
void WS_set( int mode ){
  if( WS_PIN < 0 )return;
// Если режим не поменялся  
  if( mode == ws_mode )return;
  ws_mode_save = ws_mode;
  ws_mode = mode;
//  Serial.printf("WS: mode=%d\n",ws_mode);
   switch( ws_mode ){
     case 1: //Мигнуть белым, синим, красным и вернуться к старому режиму
        for( int i=0; i<WS_PixelCount; i++ ){
           if( i%3 == 0 )strip.setPixelColor(i,strip.Color(127, 127, 127));
           else if( i%3 == 1 )strip.setPixelColor(i,strip.Color(0, 0, 255));
           else strip.setPixelColor(i,strip.Color(255, 0, 0));
        }
        strip.show();
        break;
      case 2: //Мигнуть пять раз и уйти в старый режим (настройка земли)
         for( int j=0; j<5; j++ ){
            WS_set(0,0,0);
            delay(200);
            WS_set(255,255,255); 
            delay(200);
         }
         WS_set(ws_mode_save);
         break;
       case 3: //Зажечь красный цвет (мойка занята)
         WS_set(255,0,0);            
         break;
      case 4: //Горит синим цветом (мойка свободна)
         WS_set(0,0,255);            
         break;
      case 12: //Мигнуть один раз желтым цветом и врнеуться к предыдущему (нет RTC)
         for( int j=0; j<1; j++ ){
            WS_set(0,0,0);
            delay(200);
            WS_set(255,255,0); 
            delay(200);
         }
         WS_set(ws_mode_save);
         break;
      default: 
         WS_set(0, 0, 0);
   }
  
}

void WS_set( uint8_t R, uint8_t G, uint8_t B,bool is_first){
  if( WS_PIN < 0 )return;
  if( is_first ){
     strip.setPixelColor(0,strip.Color(R,G,B));  
  }
  else {
    for( int i=1; i<WS_PixelCount; i++) strip.setPixelColor(i,strip.Color(R,G,B));
  }
  strip.show();    
}

*/

void pushRam(uint32_t _time, uint32_t _uptime, int _temp, int _hum, int _dist, bool _btn, int _stat ){
   struct EA_SaveType Val;
   Val.Time     = _time;
   Val.Uptime   = _uptime;
   Val.Temp     = _temp;
   Val.Hum      = _hum;
   Val.Distance = _dist;
   Val.Button   = _btn;
   Val.Flag     = _stat;  
   EA_Ram.push(Val);
//   Serial.printf("EA_RAM=%d\n",EA_Ram.count());
}

/**
 * Отработка дистанции сонара
 */
void ProcessingDistance(){
// Опрашиваем ультразвуковой датчик
   uint32_t _ms = millis();
   GetDistance();  
//   L = NAN;
   Distance = L;
//   Serial.printf("Distance = %d ",(int)Distance);  
//   Serial.println(L); 

   if( isnan(L) ){
//      ledSetBaseMode(LED_BASE_NAN);
//      Serial.print("Value is NAN: ");
      switch(EA_Config.NanValueFlag){
         case NAN_VALUE_IGNORE: 
            Serial.println("!!! NAN. Skiping");
            return;  
         case NAN_VALUE_BUSY:
            Serial.println("!!! NAN. BUSY");
            Button = SonarGroudState;
            break;
         case NAN_VALUE_FREE:
            Serial.println("!!! NAN. FREE");
            Button = !SonarGroudState;
            break;         
      } 
      Distance = NAN;
   }
  else {
      Distance = L;
#ifdef SONAR_FAKE
      FSonar.Set(Distance);
      if( Distance <  EA_Config.LimitDistance )Distance = EA_Config.ZeroDistance;
      else if( FSonar.Check() )Distance = EA_Config.ZeroDistance;
#endif   
//      if( Distance == 0 && EA_Config.ZeroDistance )Distance = EA_Config.ZeroDistance;
//      Serial.printf("Distance = %d\n",Distance);

// Исправление от 28 марта (Версия 4.0.3)    
// Исправление от 02.04.24 (Версия 5.0.5)    
      if( (EA_Config.MeasureType == MEASURE_TYPE_NORMAL && abs(EA_Config.GroundLevel - Distance ) < EA_Config.LimitDistance ) ||
          (EA_Config.MeasureType == MEASURE_TYPE_OUTSIDE &&  (Distance < EA_Config.MinDistance1 || Distance > EA_Config.MaxDistance1) ) ||
          (EA_Config.MeasureType == MEASURE_TYPE_INSIDE &&  (Distance > EA_Config.MinDistance2 && Distance < EA_Config.MaxDistance2) ) ){
//           abs(EA_Config.GroundLevel - Distance ) < EA_Config.LimitDistance )
           Button = SonarGroudState;
          }
       else Button = !SonarGroudState;
  }
// Зафиксировано изменение состояния   
   if( Button != LastButton ){
// Если кнопка включена       
      if( Button ){
// Если не прошел таймаут от предыдущего состояния      
         if( stat2 == STAT_OFF ){
             stat2 = STAT_BT_ON;
             ButtonTime = _ms;  
             Serial.println(F("--->Button ON"));
         }
         else {
             stat2 = STAT_OFF;
             Serial.println(F("--->Reset Button ON"));
         }
      }
      else {
         if( stat2 == STAT_OFF ){
             stat2 = STAT_BT_OFF;
             ButtonTime = _ms;  
             Serial.println(F("--->Button OFF"));
         }
         else {
             stat2 = STAT_OFF;
             Serial.println(F("--->Reset Button OFF"));
         }
      }
      LastButton = Button;
   } 
//   WS_setDistance();
   uint32_t _tm = 0;
   if( stat2 == STAT_BT_ON )  _tm = (uint32_t)EA_Config.TM_ON*1000;
   if( stat2 == STAT_BT_OFF ) _tm = (uint32_t)EA_Config.TM_OFF*1000;
//   Serial.printf("stat2=%d _tm = %ld\n",stat2,_tm);
   if( ( stat2 == STAT_BT_ON || stat2 == STAT_BT_OFF ) && (_tm == 0 || _ms < ButtonTime || _ms - ButtonTime >= _tm ) ) {
       Serial.println(F("--->Fixed Button"));
       stat2 = STAT_OFF;
       Relay_setDistance();
       pushRam(Time, _ms/1000, Temp, Hum, Distance, Button, 0 );
   }
  
}

void WS_setDistance(){
  if( isnan(Distance) ){
      switch(EA_Config.NanValueFlag){
         case NAN_VALUE_IGNORE: ledSetBaseMode(LED_BASE_NAN); break;
         case NAN_VALUE_BUSY:   ledSetBaseMode(LED_BASE_NAN_BUSY); break;
         case NAN_VALUE_FREE:   ledSetBaseMode(LED_BASE_NAN_FREE); break;
      } 



     
//     Serial.println("Led NAN");
  }
  else {
     if( Button )ledSetBaseMode(LED_BASE_BUSY);
     else ledSetBaseMode(LED_BASE_FREE);
  }
}
      
void Relay_setDistance(){
  if( Button ){
     if( PinRelay>=0 )digitalWrite(PinRelay,HIGH);
     if( PinController>=0 )digitalWrite(PinController,HIGH);  
  }
  else {
     if( PinRelay>=0 )digitalWrite(PinRelay,LOW);
     if( PinController>=0 )digitalWrite(PinController,LOW);    
  }
}

bool scanI2C(int _addr){
   Serial.println("Scanning...");
   bool ret = false;
   int nDevices = 0;
   for(int address = 1; address < 127; address++ ){
      Wire.beginTransmission(address);
      int error = Wire.endTransmission();       
      if (error == 0){
         if( _addr == address )ret = true;
         Serial.printf("I2C device 0x%02x\n",address);
         nDevices++;
      }
   }
   if (nDevices == 0)
     Serial.println("No I2C devices found\n");
   else
     Serial.println("done\n");  
   return ret;  
}

void lidarSetI2C(){
   lidarTF.begin(115200);
   uint8_t tx_buf[20];
   tx_buf[0] = 0x5A;
   tx_buf[1] = 0x05;
   tx_buf[2] = 0x0A;
   tx_buf[3] = 0x01;
   tx_buf[4] = 0x6A;
   Serial.println("Lidar set I2C mode");
   lidarTF.write(tx_buf,5);
   lidarTF.flush();
   tx_buf[0] = 0x5A;
   tx_buf[1] = 0x04;
   tx_buf[2] = 0x11;
   tx_buf[3] = 0x6F;
   Serial.println("Lidar save setting");
   lidarTF.write(tx_buf,4);
   lidarTF.flush();
//   ESP.restart();
}


SonarFake::SonarFake(uint16_t d){
   if( d == 0 || d > DEPTH_DIST_ARRAY)depth = DEPTH_DIST_ARRAY;
   else depth = d;
   data = (int *)malloc(sizeof(int)*depth);
   first = true;
   cur   = 0; 
}

void SonarFake::Set( int _val ){
   if( first )  {
      for( int i=0; i<depth; i++ )data[i] = _val;
      cur=0;
      first = false;  
   }
   else  {
      for( int i=depth-1; i>0; i--)data[i] = data[i-1];
      data[0]=_val;
   }
   for( int i=0; i<depth; i++ ){
      Serial.printf("%d ",data[i]);      
   }
   Serial.println();   
}

// Проверка на убывающую функцию
bool SonarFake::Check(){
   bool ret = true; 
   for( int i=0; i<depth-1; i++){
       if(data[i] >= data[i+1])ret = false;      
   }
   return ret;
}

void PrintValue(){
         Serial.print("!!! Value: Time=");    
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
         Serial.println(LastButton);
}

/**
* Автокалибровка земли
*/
bool CalibrateGround(){
   ledSetBaseMode(LED_BASE_NONE);
   delay(EA_Config.TM_BEGIN_CALIBRATE*1000);
   ledSetBaseMode(LED_BASE_GROUND);
   int n = 0;
   float x = 0.0;
   for( int i=0; i<EA_Config.SAMPLES_CLIBRATE; i++){
      ProcessingDistance();
      if( !isnan(Distance) ){
        x += Distance;
        n++;
        PrintValue();
      }
      delay(300);
   }
//   ledRestoreMode();
   if( n>0 ){
      x /= n;
      Serial.printf("!!! Calibrate ground value %d\n",(int)x);
      EA_Config.GroundLevel  = (int)x;
      EA_Config.MinDistance1 = (int)x;
      EA_Config.MaxDistance1 = (int)x;
      EA_Config.MinDistance2 = (int)x;
      EA_Config.MaxDistance2 = (int)x;
      return true;
   }
   else {
      Serial.println("??? Calibrate ground fail");
      return false;
   }
}