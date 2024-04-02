/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/

#ifndef Archive_h
#define Archive_h
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "src/StackArray.h"
#include <arduino.h>
#include "Config.h"

struct EA_ConfigType{
   char ESP_NAME[32];
//   char ESP_PASS[32];
   char AP_SSID[32];
   char AP_PASS[32];
   bool isDHCP;
   bool isWiFiAlways;
   IPAddress IP;
   IPAddress MASK;
   IPAddress GW;
   IPAddress DNS;
   char ESP_ADMIN_PASS[32];
   char ESP_OPER_PASS[32];
   char DOGOVOR_ID[16]; 
   char BOX_ID[8];     
   char SERVER[32];  
   uint16_t PORT;
   int GroundLevel;   
   int LimitDistance;
   int MinDistance;
   int MaxDistance;
//   int LimitDistanceUp;
   int ZeroDistance;
   uint8_t TM_ON; // Задержка включения, сек
   uint8_t TM_OFF; //Задержка выключения, сек
   uint8_t TM_BEGIN_CALIBRATE;
   uint8_t SAMPLES_CLIBRATE;
   T_SENSOR_TYPE SensorType;
   T_NAN_VALUE_FLAG NanValueFlag;
   T_MEASURE_TYPE MeasureType;
//   bool isAP;
   uint16_t SRC;   
};

/**
 * Оганизация архива значений в виде стека в EEPROM 
 */

#define EA_VALUE_NUMBER 300  //Число значений архива (300)
#define EA_BUFFER_SIZE  10   //Максимальное число значений читаемых из архива за раз
#define EA_OFFSET       256  //Начальное смещение архива во флэш-памяти


// Одно значение архива 12 байт 
struct EA_Value {
    uint32_t Time;     //Время записи
    int      Temp;     //Значение температуры, C
    int      Hum;      //Значение влажности, %
    int      Distance; //Значение дистанции, мм 
    bool     Button;   //Состояние кнопки;
    bool     Enable;   // Состояние записи архива
};

// Cохраняемые текущие значения
struct EA_SaveType {
    uint32_t Time;     //Время записи
    uint32_t Uptime;   //Время аптайма
    int      Temp;     //Значение температуры, C
    int      Hum;      //Значение влажности, %
    int      Distance; //Значение дистанции, мм 
    bool     Button;   //Состояние кнопки
    int      Flag;     //Состояние ESP
};

extern uint16_t      EA_Count;
extern size_t        EA_Size;
extern size_t        EA_Offset;
extern EA_Value      EA_Buffer[EA_BUFFER_SIZE];
extern EA_SaveType   EA_Save;
extern EA_ConfigType EA_Config;
extern StackArray <struct EA_SaveType> EA_Ram;

extern char SensorID[];



void EA_begin(void);
void EA_get_count(void);
void EA_save(uint32_t tm,int t,int h,int d,bool bt,int check_interval=0);
void  EA_get(int n,struct EA_Value &val);

int  EA_read10();
void EA_shift( int cnt );
void EA_save_last(uint32_t tm,uint32_t uptime,int t,int h,int d,bool bt,int flag);
void EA_read_last();
void EA_clear_arh();
void EA_print_arh();
void EA_read_config();
void EA_save_config();

uint16_t EA_SRC(void);
void     EA_default_config(void);

extern const char *_VERSION;

#endif
