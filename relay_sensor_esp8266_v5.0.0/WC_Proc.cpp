/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#include "WC_PROC.h"
#include "Config.h"
#include "Archive.h"

int   Temp         = 0;
int   LastTemp     = 0;
int   Hum          = 0;
int   LastHum      = 0;
int   LastDistance = 0;
int   Distance     = 0;
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



uint8_t ws_mode   = 0;
int      ws_mode_save  = -1;
uint16_t ws_tm    = WS_TM_DEFAULT;
int      ws_stat  = 0;
//strip.Color ws_color = strip.Color(0, 0, 0), ws_color_save;
bool ws_enable = true;


bool RTCFlag  =  true;
bool FlagWDT  =  false;
RTC_DS3231 rtc;
DHT dht(PinDHT22, DHT22);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(WS_PixelCount, WS_PIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial ultraSensor(PinDistanceEcho,-1);
SoftwareSerial lidarTF(PinDistanceTrig,PinDistanceEcho);
SButton calbtn( PinCalibrateSonar );
TFMPI2C tfmP;   
TFLI2C tflI2C;

/**
 * Инициализация ультразвукового датчика
 */
void InitSonar(){
// Иницализация ультразвукового датчика
   if( SONAR_SENSOR_TYPE == SONAR_TRIG_ECHO || 
     SONAR_SENSOR_TYPE == SONAR_JSN_SR04TV2 ||
     SONAR_SENSOR_TYPE == SONAR_JSN_SR04M_2 ){
      if( PinDistanceTrig >= 0 ){
         Serial.printf("Init Sonar trig=%d, echo=%d ...\n",
            PinDistanceTrig,PinDistanceEcho);
         pinMode(PinDistanceTrig , OUTPUT);
         pinMode(PinDistanceEcho , INPUT);
         digitalWrite(PinDistanceEcho , HIGH);         
      }
      else {
         Serial.printf("Sonar is disabled\n");
      }
   }
   else if( SONAR_SENSOR_TYPE == SONAR_SERIAL ){
      if( PinDistanceEcho >= 0 ){
//         ultraSensor.begin(9600);
         Serial.printf("Init Sonar serial type RX=%d ...\n",  PinDistanceEcho);
      }
      else {
         Serial.printf("Sonar is disabled\n");
      }
      
   }
   else if( SONAR_SENSOR_TYPE == LIDAR_TFMINI_I2C ){
//     scanI2C();
//     tfmP.recoverI2CBus();
//     Wire.stop();
     Wire.begin(PinDistanceEcho,PinDistanceTrig);
     if( scanI2C(0x10) == false ){
         Serial.println("Test UART LiDAR");
//         Wire.end();
         lidarSetI2C();
         Wire.begin(PinDistanceEcho,PinDistanceTrig);
         scanI2C(0x10);
     }
      if( !tfmP.sendCommand( SOFT_RESET, 0) ){
         Serial.println("ERR ");
         tfmP.printReply();
      }
      else Serial.println("OK");
    
   }
   else if( SONAR_SENSOR_TYPE == LIDAR_TFLUNA_I2C ){
//     scanI2C();
//     tfmP.recoverI2CBus();
//     Wire.stop();
     Wire.begin(PinDistanceEcho,PinDistanceTrig);
     scanI2C(0x10);
     Serial.println("Test TFLuna ");
      if( !tflI2C.Soft_Reset(0x10) ){
         Serial.println("ERR ");
         tflI2C.printStatus();
      }
      else Serial.println("OK");
    
   }
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
   if( SONAR_SENSOR_TYPE == SONAR_TRIG_ECHO ){
      if( PinDistanceTrig < 0 )return;
//      ws_enable = false;
//      digitalWrite(PinDistanceTrig, LOW);
//      delayMicroseconds(2);
      digitalWrite(PinDistanceTrig, HIGH);
      delayMicroseconds(10);
      digitalWrite(PinDistanceTrig, LOW);
//      noInterrupts();
//      delayMicroseconds(2);
      float d = pulseIn(PinDistanceEcho, HIGH); // max sensor dist ~4m
//      interrupts();
      Distance =  d / 5.8;
//      Distance =  d / 5.88235;
      
//      long impulseTime  = pulseIn(PinDistanceEcho, HIGH);
//      long impulseTime  = pulseIn(PinDistanceEcho, HIGH,100000);
//      Distance   = (int)(impulseTime / 5.8);
   }
   else if( SONAR_SENSOR_TYPE == SONAR_JSN_SR04TV2 ){
      digitalWrite(PinDistanceTrig, LOW);
      delayMicroseconds(2);
      digitalWrite(PinDistanceTrig, HIGH);
      delayMicroseconds(10);
      digitalWrite(PinDistanceTrig, LOW);  
      const unsigned long duration= pulseIn(PinDistanceEcho, HIGH);
      Distance = (int)((float)duration/5.8);  
   }  
   else if( SONAR_SENSOR_TYPE == SONAR_JSN_SR04M_2 ){
      Distance = 0;
      int _count = 0;
      for( int i=0; i<5; i++){
          digitalWrite(PinDistanceTrig, LOW);
         delayMicroseconds(2);
         digitalWrite(PinDistanceTrig, HIGH);
         delayMicroseconds(500);
         digitalWrite(PinDistanceTrig, LOW);  
         const unsigned long duration= pulseIn(PinDistanceEcho, HIGH);
         float x = (int)((float)duration/5.8);
         if( x>0 && x < 5000){
            Distance += x;
            _count++;
         }
         delay(50);
      }  
      if( _count > 0)Distance /= _count;
   }
   else if( SONAR_SENSOR_TYPE == SONAR_SERIAL ){
      if( PinDistanceEcho < 0 )return;
      Distance   = GetDistanceSerial();
   }
   else if( SONAR_SENSOR_TYPE == LIDAR_TFLUNA_I2C ){
      int16_t _dist, _flux, _temp;
     if( tflI2C.getData( _dist, _flux, _temp, 0x10) ){
         Serial.printf("TFLuna dist=%d flux=%d temp=%d\n",
             (int)_dist, (int)_flux, (int)_temp);
         Distance = _dist*10;    
      }
      else {
         Serial.printf("Error TF Luna dist\n");
         Distance = -1;    
      }
         
   }
   else if( SONAR_SENSOR_TYPE == LIDAR_TFMINI_I2C ){
      int16_t _dist, _flux, _temp;
      tfmP.getData( _dist, _flux, _temp);
      if( tfmP.status == TFMP_READY){
         Serial.printf("TFMini dist=%d flux=%d temp=%d\n",
             (int)_dist, (int)_flux, (int)_temp);
         Distance = _dist*10;    
      }
      else {
         Serial.printf("Error LiDAR dist\n");
         Distance = -1;    
      }
         
   }
}


/**
 * Опрос ультразвукового датчика с мигающим светодиодом
 */
int GetDistanceSerial(){

  
   byte readByte;
   byte crcCalc;
   word distance = 0;

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
     if( crcCalc == b3 )distance = (b1 * 0xff) + b2;
  }
  ultraSensor.flush();
  ultraSensor.end();
  return distance;

  

  }


int GetDistanceSerial1(){
  
   uint32_t ms_first  = millis();
   uint32_t ms_last  = ms_first;
   uint8_t b = 0,byte_count = 0;
   byte crc  = 0;
   int  val = -1;
   int  val_avg = 0;
   int  val_count = 0;
   ultraSensor.begin(9600);
   Serial.printf("Get distance\n");
   for( val_count=0; val_count<1000 && (ms_last-ms_first)<1000;){
       ESP.wdtFeed();
       if( ultraSensor.available() < 1 ){
           b = ultraSensor.read();
           switch(byte_count){
              case 0: //0-байт. Метка
                 if( b == 0xff){
                     crc = b;
                     byte_count++; 
                 }
                 break;
              case 1: //1-байт. Старший разряд значения
                 val = b*0xff;
                 crc += b;
                 byte_count++;
                 break;
              case 2: //2-байт. Младший разряд значения   
                 val += b;
                 crc += b;
                 byte_count++;
                 break;
              default: //3-байт. Контрольная сумма
                 if( crc != b || val == 255 )val = -1;
                 else {
                     val_avg += val;
                     val_count++;  
                 }
                 byte_count = 0;
                 break;
           }//end case
//           Serial.printf("distance = %d\n",val);
       }//end if
       ms_last=millis();
   }//end for
   if( val_count == 0 )val_avg = 0;
   else val_avg /= val_count;
   Serial.printf("Distance = %d mm\n",val_avg);
   ultraSensor.end();
   return val_avg;
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
#ifdef SONAR_FAKE
   FSonar.Set(Distance);
   if( Distance <  EA_Config.LimitDistance )Distance = EA_Config.ZeroDistance;
   else if( FSonar.Check() )Distance = EA_Config.ZeroDistance;
#endif   
   if( Distance == 0 && EA_Config.ZeroDistance )Distance = EA_Config.ZeroDistance;
   Serial.printf("Distance = %d\n",Distance);
   if( PinDistanceEcho >= 0 ){
// Исправление от 28 марта (Версия 4.0.3)    
       if( EA_Config.LimitDistanceUp >= 0 && Distance > EA_Config.LimitDistanceUp ||
           abs(EA_Config.GroundLevel - Distance ) < EA_Config.LimitDistance )
//      if( Distance > EA_Config.GroundLevel || abs(EA_Config.GroundLevel - Distance ) < EA_Config.LimitDistance )
            Button = SonarGroudState;
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
   uint32_t _tm = 0;
   if( stat2 == STAT_BT_ON )  _tm = (uint32_t)EA_Config.TM_ON*1000;
   if( stat2 == STAT_BT_OFF ) _tm = (uint32_t)EA_Config.TM_OFF*1000;
//   Serial.printf("stat2=%d _tm = %ld\n",stat2,_tm);
   if( ( stat2 == STAT_BT_ON || stat2 == STAT_BT_OFF ) && (_tm == 0 || _ms < ButtonTime || _ms - ButtonTime >= _tm ) ) {
       Serial.println(F("--->Fixed Button"));
       stat2 = STAT_OFF;
       WS_setDistance();
       Relay_setDistance();
       pushRam(Time, _ms/1000, Temp, Hum, Distance, Button, 0 );
   }
  
}

void WS_setDistance(){
  if( Button )WS_set(3);
  else WS_set(4);
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
