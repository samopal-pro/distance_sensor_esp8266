#ifndef WC_CONFIG_h
#define WC_CONFIG_h
#include "MyConfig.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>


#define CONFIG_JSON     "/config.json"
#define SAVE_JSON       "/save.json"

extern JsonDocument jsonConfig;
extern JsonDocument jsonSave;
extern bool isChangeConfig;

typedef enum T_AP_START_MODE {
  AP_NONE           = 0, //Не стартовать AP при загрузке
  AP_ALWAYS         = 1, //Всегда стартовать AP при загрухке
  AP_FIRST          = 2  //Стартовать AP при первой загрузке
};


enum T_INSTALL_TYPE {
  INSTALL_TYPE_NORMAL  = 1, //Срабатываение датчика в обычном режиме (уменьшение дистанции)
  INSTALL_TYPE_OUTSIDE = 2, //Срабатывание датчика если вне интервала
  INSTALL_TYPE_INSIDE  = 3  //Срабатывание датчика если внутри интервала
};

enum T_NAN_VALUE_FLAG {
  NAN_VALUE_IGNORE  = 1,  
  NAN_VALUE_FREE    = 2,
  NAN_VALUE_BUSY    = 3
};

enum T_RELAY_MODE {
  RELAY_NONE       = 0, //Реле отключено
  RELAY_NORMAL     = 1, //Реле работает на ON/OFF
  RELAY_PULSE      = 2, //Реле при срабатывание выдает имульс на заданное время при включении или выключении
  RELAY_PULSE_OFF  = 3, //Реле при сбрасываниее выдает имульс на заданное время (не используется)
  RELAY_PWM        = 4,  //Реле при срабатывание выдает имульсы с заданной длительностью и паузой
  RELAY_PULSE2     = 5  //Реле при срабатывание выдает имульс на заданное время при включение и выключении
};

void configInit();
void configRead();
void configSave();
void configDefault();
void configPrint(char *label);

void saveRead();
void saveSave();
void saveDefault();
void savePrint(char *label);
void saveSet(float _dist, SENSOR_STAT_t _on);
uint16_t saveCount();


void listDir(const char * dirname, uint8_t levels=0);
#endif
