#ifndef WC_SENSORS_h
#define WC_SENSORS_h
#include <Wire.h>
#include "src/QuickStats/QuickStats.h"
#include "MyConfig.h"
#include "WC_Config.h"

typedef enum {
   ST_NONE      = 0,
   ST_RANGE     = 1,   //Датчик дистанции
   ST_TEMP      = 2,   //Датчик температуры
   ST_OTHER     = 100  //Прочий тип датчиков

} SensorValueType_t;

#if (DEFAULT_SENSOR_TYPE == SENSOR_SR04T )||(DEFAULT_SENSOR_TYPE == SENSOR_SR04TM2 )
#include "src/SonarSR04/SonarSR04.h"
//#include "src/SonarA21/SonarA21.h"
#elif ( DEFAULT_SENSOR_TYPE == SENSOR_TFLUNA_I2C )
#include "src/TF/TFLI2C.h"
//#include "src/TF/TFMPI2C.h"
#endif
class MySensorValue {
   public:
      MySensorValue(SensorValueType_t _type, String _label, float _min, float _max, float _error = 0, uint16_t _mult=1, uint16_t _samples = 10);
      void clear();
      bool set( float _val, bool flag = true);
      float getLast();
      float getAverage();
      float getDeviation();
      SensorValueType_t Type;
      String Label;
      float    Value;
      uint16_t Multiplier;   
   private:
      float    LimitMin;
      float    LimitMax;
      float    ValueError;
      uint16_t Samples;
      bool     isFirst;
      uint8_t  Pointer;
      float    *Values;
};

class MySensor {
   public:
      String Name;
      MySensor();
      bool init();
      bool get();
      uint16_t count_test;
      MySensorValue *Value;
   private:
      bool isInit;
      void *Sensor;
//      TFLI2C *SensorTFLuna; 
      bool checkI2C(uint8_t _addr);   
      void scanI2C();
};

#define NTC_SAMPLE          10
#define NTC_DELAY_SAMPLE    10
#define NTC_DELAY           100

class TempNTC {
   public: 
      TempNTC(uint8_t _pin, float _b=3950.0, float _refU=2500.0, float _refR=10000.0, float  _nomR=100000.0, float _nomT=25.0);
      float get();
   private:
      uint8_t Pin;
      float ReferenceR;
      float NominalR;
      float ReferenceU;
      float NominalT;
      float B;
      float calc( float _x);
};


#endif
