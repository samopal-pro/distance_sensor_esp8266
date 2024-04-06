/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#ifndef WS_PROC_h
#define WS_PROC_h
#include <arduino.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#include "src/Adafruit_NeoPixel.h"  // https://github.com/adafruit/Adafruit_NeoPixel
#include "src/DHT.h"                // https://github.com/adafruit/DHT-sensor-library
//#include "src/RTClib.h"             // https://github.com/adafruit/RTClib
#include "src/TimeLib.h"            //https://playground.arduino.cc/Code/Time/
#include "src/SButton.h"
#include "src/TFMPI2C.h"            // https://github.com/budryerson/TFMini-Plus-I2C
#include "src/TFLI2C.h"             // https://github.com/budryerson/TFLuna-I2C
#include "WC_Led.h"
#define DEPTH_DIST_ARRAY 5
#define SAMPLE_LEN       10
#define RELIABILITY_PROC 0.15




#define ReceiveBuferSize         1024
#define SendBuferSize            10240
/*
extern int   Temp;
extern int   LastTemp;
extern int   Hum;
extern int   LastHum;
extern int   LastDistance;
extern int   Distance;
*/
extern float Temp, LastTemp, Hum, LastHum, LastDistance, Distance;
extern bool  Button;
extern bool  LastButton;

extern bool  PinCalibrateSonarState;
extern bool  isAP;
extern char  sbuf[];
extern char  rbuf[];

extern unsigned long Time;
extern unsigned long RTC_Time;
extern unsigned int DO;
extern bool   WDT_active;
extern int     FlagStat;
extern unsigned int ErrCount;
extern unsigned int TryCount;
extern bool     ResetByErr;
extern const char KeyGenStrParam[];
extern char SensorID[];

extern unsigned long ButtonTime;

enum ES_STAT {
  STAT_OFF,
  STAT_BT_ON,
  STAT_BT_OFF,
  STAT_WAIT_ON,
  STAT_WAIT_OFF
};

extern ES_STAT stat2;
extern bool is_first_flag;
extern unsigned long t_ws_send;
extern unsigned long t_ws_recv;

extern uint8_t ws_mode;
extern int      ws_mode_save;
extern uint16_t ws_tm;
extern int      ws_stat;
//extern  RgbColor ws_color, ws_color_save;
extern bool ws_enable;

extern bool RTCFlag;
extern bool FlagWDT;
//extern RTC_DS3231 rtc;
extern DHT dht;
extern SButton calbtn;



bool CheckTime(time_t t);
void InitSonar();
void GetDistance();

void GetDistanceSR04(uint32_t tm1=2, uint32_t tm2=10, int samples=10, float min=NAN, float max=NAN);
void GetDistanceTFMini();
void GetDistanceTFLuna();
void GetDistanceSerial();
bool HttpGetStatus(void);
bool HttpSetParam(uint32_t _time, uint32_t _uptime, int _temp, int _hum, int _dist, bool _btn, int _stat );
bool HttpSetStatus(uint32_t _time, uint32_t _uptime, int _temp, int _hum, int _dist );
uint16_t KeyGen();
bool HttpArchiveParam();
//void WDT_init();
//void WDT_disable();
//void WDT_reset();
//unsigned long GetRTClock();
//void SetRTClock(unsigned long Time);
//void WS_set( int mode );
void WS_set( uint8_t R, uint8_t G, uint8_t B,bool is_first = false );
void pushRam(uint32_t _time, uint32_t _uptime, int _temp, int _hum, int _dist, bool _btn, int _stat );

void ProcessingDistance();
void PrintValue();
void WS_setDistance();
void Relay_setDistance();
void PrintTime( time_t t );
bool scanI2C(int _addr);
void lidarSetI2C();
float CalibrateGround();
bool ProcessingCalibrate();


class SonarFake {
   private:
      int *data;
      uint8_t cur;
      uint16_t depth;  
      bool first;
   public:
      SonarFake(uint16_t d=DEPTH_DIST_ARRAY);
      void Set(int _val);  
      bool Check();
};

#endif
