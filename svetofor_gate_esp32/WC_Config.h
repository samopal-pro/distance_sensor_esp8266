#ifndef WC_CONFIG_h
#define WC_CONFIG_h
#include "MyConfig.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>


#define CONFIG_JSON        "/config.json"
#define NODE_LIST_JSON     "/nodelist.json"
#define GATEWAY_LIST_JSON  "/gatelist.json"
//#define SAVE_JSON       "/save.json"

typedef enum T_AP_START_MODE {
  AP_NONE           = 0, //Не стартовать AP при загрузке
  AP_ALWAYS         = 1, //Всегда стартовать AP при загрухке
  AP_FIRST          = 2  //Стартовать AP при первой загрузке
};


extern JsonDocument jsonConfig;
extern JsonDocument jsonNodeList;
extern JsonDocument jsonGatewayList;

extern bool isChangeConfig;

void configInit();
void configRead();
void configSave();
void configDefault();
void configPrint(char *label);

void nodeListRead();
void nodeListSave();
void nodeListPrint(char *label);
void gateListRead();
void gateListSave();
void gateListPrint(char *label);

void listDir(const char * dirname, uint8_t levels=0);
#endif
