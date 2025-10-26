#include "WC_Sensors.h"

QuickStats qstat;

/**
* Класс MySensorValue
*/
MySensorValue::MySensorValue(SensorValueType_t _type, String _label, float _min, float _max, float _error, uint16_t _mult, uint16_t _samples){
   Type       = _type;
   Label      = _label;
   LimitMin   = _min;
   LimitMax   = _max;
   ValueError = _error;
   Samples    = _samples;
   Multiplier = _mult;
   if( Type != ST_NONE ){
      Values     = (float *)malloc(Samples*sizeof(float));
      clear();
   }
}

void MySensorValue::clear(){
   if( Type == ST_NONE )return;
   for( int i=0; i<Samples; i++)Values[i] = NAN;
   isFirst       = true;
   Pointer       = 0;
   Value         = NAN;
}

bool MySensorValue::set(float _val, bool _flag){
   if( Type == ST_NONE ){
      Value = NAN;
      return true;        
   }
   if( !isnan(_val) )_val *= Multiplier;
   if( isnan(_val) || _val < LimitMin || _val > LimitMax ){
      Value = NAN;
      return false;  
   }
   Value = _val;
   if( !_flag )return false;
   if( isFirst ){
      isFirst = false;
      for( int i=0; i<Samples; i++)Values[i] = _val;
   }
   else {
      Values[Pointer++] = _val;
      if( Pointer >= Samples )Pointer = 0;
   }
   return true;
}


float MySensorValue::getLast(){
   if( Type == ST_NONE )return ValueError;
   if( isFirst )return ValueError;
   if( isnan(Value ) )return ValueError;
   return ( Value);
}

float MySensorValue::getAverage(){
   if( Type == ST_NONE )return ValueError;
   if( isFirst )return ValueError;  
   float _val = qstat.average(Values,Samples);
   if( isnan(_val) )return ValueError;
   else return (_val );
}

float MySensorValue::getDeviation(){
   if( Type == ST_NONE )return 0;
   if( isFirst )return 0;
   float _val = qstat.stdev(Values,Samples);
   if( isnan(_val) )return 0;
   else return (_val * 100.0);
 }

/**
**************************************************************************************************************************************************************************************
* Класс MySensor
*/

MySensor::MySensor(){
   bool isInit;

   switch(jsonConfig["SENSOR"]["TYPE"].as<int>()){
      case SENSOR_SR04T :  
//#if (DEFAULT_SENSOR_TYPE == SENSOR_SR04T )

         Sensor = new SonarSR04(PIN_SONAR_ECHO, PIN_SONAR_TRIG, 2, 10);
         Value = new MySensorValue(ST_RANGE,"Дистанция, мм", 100.0, 5000.0, NAN, 1, SIMPLE_SIZE );
         Name   = "Sonar SR04T";
         Serial.println("!!! new Sonar SR04T");
         break;
//#elif (DEFAULT_SENSOR_TYPE == SENSOR_SR04TM2 )

      case SENSOR_SR04TM2 :  
         Sensor = new SonarSR04(PIN_SONAR_ECHO, PIN_SONAR_TRIG, 2, 500);
         Value = new MySensorValue(ST_RANGE,"Дистанция, мм", 100.0, 7500.0, NAN, 1, SIMPLE_SIZE );
         Name   = "Sonar SR04TM2";
         Serial.println("!!! new Sonar SR04TM2");
         break;
//      case SENSOR_A21_I2C:   
//         Sensor = new SonarA21();
//         Value = new MySensorValue(ST_RANGE,"Дистанция, мм", 100.0, 5000.0, NAN, 1, SIMPLE_SIZE );
//         Name   = "Sonar A21 I2C";
 //        break;
//#elif (DEFAULT_SENSOR_TYPE == SENSOR_TFLUNA_I2C )
      case SENSOR_TFLUNA_I2C :
         Sensor = new TFLI2C();
         Value = new MySensorValue(ST_RANGE,"Дистанция, мм", 100.0, 8000.0, NAN, 10, SIMPLE_SIZE );
         Name   = "Lidar TFLuna I2C";
         Serial.println("!!! new Lidar TFLuna");
         break;
      case SENSOR_TFMINI_I2C :
         Sensor = new TFMPI2C();
         Value = new MySensorValue(ST_RANGE,"Дистанция, мм", 100.0, 12000.0, NAN, 10, SIMPLE_SIZE );
         Name   = "Lidar TFMiniPlus I2C";
         break;
      case SENSOR_LD2413_UART :
         Sensor = (LD2413 *)new LD2413();
         Value = new MySensorValue(ST_RANGE,"Дистанция, мм", 150.0, 10000.0, NAN, 1, 1);
         Name   = "Датчик HiLINK LD2413 (UART)";
         break;

   }
//#endif

//   Serial.println("!!! ???");

}

bool MySensor::init(){
   switch(jsonConfig["SENSOR"]["TYPE"].as<int>()){
      case SENSOR_SR04T :  
      case SENSOR_SR04TM2 :  
//#if (DEFAULT_SENSOR_TYPE == SENSOR_SR04T )||(DEFAULT_SENSOR_TYPE == SENSOR_SR04TM2 )
         ((SonarSR04 *)Sensor)->init();
         break;


/*
      case SENSOR_A21_I2C:   
//   Wire.begin();
         pinMode(PIN_I2C_SCL, OUTPUT);
         digitalWrite(PIN_I2C_SCL,LOW);
         pinMode(PIN_I2C_SDA, OUTPUT);
         digitalWrite(PIN_I2C_SDA,LOW);
 
         Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
//   Wire.setClock(100000);
         vTaskDelay(200);
         scanI2C();


         isInit = ((SonarA21 *)Sensor)->init();
#if (DEBUG_SENSORS>0)
         Serial.printf("!!! Init I2C %d %d\n",PIN_I2C_SDA, PIN_I2C_SCL);
#endif
         break;
*/
      case SENSOR_TFLUNA_I2C:
//#elif (DEFAULT_SENSOR_TYPE == SENSOR_TFLUNA_I2C )
         Wire.begin(PIN_SONAR_ECHO, PIN_SONAR_TRIG);
         Wire.setClock(100000);
         vTaskDelay(200);
         isInit = checkI2C(TFL_DEF_ADR);
#ifdef DEBUG_SENSORS
         if( isInit )Serial.println(F("!!! Init TFLuna"));
         else Serial.println(F("!!! Error TFLuna init"));
#endif
         break;
      case SENSOR_TFMINI_I2C:
         Wire.begin(PIN_SONAR_ECHO, PIN_SONAR_TRIG);
   Wire.setClock(100000);
//         vTaskDelay(200);
         isInit = checkI2C(TFMP_DEFAULT_ADDRESS);
#ifdef DEBUG_SENSORS
         if( isInit )Serial.println(F("!!! Init TFMiniPlus"));
         else Serial.println(F("!!! Error TFMiniPlus init"));
#endif
         break;
      case SENSOR_LD2413_UART :
         Serial2.begin(115200,SERIAL_8N1,PIN_SONAR_TRIG,PIN_SONAR_ECHO);
         ((LD2413 *)Sensor)->begin(PIN_SONAR_TRIG,PIN_SONAR_ECHO);
         ((LD2413 *)Sensor)->init(150, 10000, 250);
         isInit = true;

   }
//#endif
#if (DEBUG_SENSORS>0)
   if( isInit )Serial.print(F("!!! Init normal "));
   else Serial.print(F("??? Init fail "));
   Serial.println(Name);
#endif



//   isInit = true;
   return isInit;
}


bool MySensor::get(){
   float value1 = NAN, value2 = NAN;
   bool stat = false;
   if( !isInit )init();
   if( !isInit )return false;
   static int16_t _dist16, _flux16, _temp16;
//#if (DEFAULT_SENSOR_TYPE == SENSOR_SR04T )||(DEFAULT_SENSOR_TYPE == SENSOR_SR04TM2 )
   switch(jsonConfig["SENSOR"]["TYPE"].as<int>()){
      case SENSOR_SR04T:
      case SENSOR_SR04TM2:
         value1 = ((SonarSR04 *)Sensor)->getDistance();
         stat = Value->set(value1,true);
         break;
//      case SENSOR_A21_I2C:
//         value1 = (float)((SonarA21 *)Sensor)->getDistance();
//         stat = Value->set(value1);
//         break;
      case SENSOR_TFLUNA_I2C:    
//#elif (DEFAULT_SENSOR_TYPE == SENSOR_TFLUNA_I2C )
//         stat = SensorTFLuna->getData(_dist16,TFL_DEF_ADR);
         stat = ((TFLI2C *)Sensor)->getData(_dist16, _flux16, _temp16, TFL_DEF_ADR );
         if( stat ) value1 = (float)_dist16;
         else value1 = NAN;
         Value->set(value1, stat);
         break;
      case SENSOR_TFMINI_I2C:    
         stat = ((TFMPI2C *)Sensor)->getData(_dist16, _flux16, _temp16, TFMP_DEFAULT_ADDRESS );
         if( stat ) value1 = (float)_dist16;
         else value1 = NAN;
         Value->set(value1, stat);
         break;
      case SENSOR_LD2413_UART :
         value1 = ((LD2413 *)Sensor)->wait_data();
         if( value1!= 0 )Value->set(value1);
#if DEBUG_SENSORS > 1
         Serial.print(F("!!! LD2413 value: "));
         Serial.println(value1,1);
#endif
         break;
   }
//#endif
#if (DEBUG_SENSORS>1)
   Serial.print(F("!!! "));
   Serial.print(Name);
   Serial.print(F(" = "));
   Serial.println(Value->getLast());
//   Serial.println(value1);
#endif 

return stat;
}

/**
// Проверка I2C адреса
*/
bool MySensor::checkI2C(uint8_t _addr){
   bool _ret = false;
   Wire.beginTransmission(_addr);
   if( Wire.endTransmission() == 0 )_ret = true;
   else _ret = false;
#ifdef DEBUG_SENSORS
   Serial.print(F("!!! I2C: ADDR=0x"));
   Serial.print(_addr,HEX);   
   if( _ret )Serial.println(F(" OK"));
   else Serial.println(F(" False"));
#endif  
   return _ret;
}

void MySensor::scanI2C(){
  int nDevices = 0;
  for(uint8_t address = 1; address < 127; address++ ) {
     Wire.beginTransmission(address);
     int error = Wire.endTransmission();
     switch(error){
         case 0 :
            Serial.printf(F("!!! I2C device found at address 0x"));
            if (address<16)Serial.print("0");
            Serial.println(address,HEX);
            nDevices++;
            break;
         case 4:
            Serial.print("Unknow error at address 0x");
            if (address<16)Serial.print("0");
            Serial.println(address,HEX);
            break;
      }    
  }
  if (nDevices == 0)Serial.println(F("No I2C devices found\n"));
}


