/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/

#ifndef WS_HTTP_h
#define WS_HTTP_h
#include <arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <DNSServer.h>

#include <ESP8266HTTPUpdateServer.h>
#include "Archive.h"
#include "WC_Led.h"
#include "WC_Proc.h"

#define TM_AP_MODE      120000
#define TM_WIFI_CONNECT 20000

extern ESP8266WebServer server;
extern char SensorID[];
extern bool isAP;
extern float Temp, LastTemp, Hum, LastHum, LastDistance, Distance;
extern unsigned long Time;
extern uint32_t msAP,msSTA;

enum ES_WIFI_STAT {
  EWS_OFF,
  EWS_AP_MODE,
  EWS_NOT_CONFIG,
  EWS_WAIT,
  EWS_ON
};

typedef enum {
  HT_TEXT     = 0,
  HT_PASSWORD = 1,
  HT_NUMBER   = 2,
  HT_IP       = 3,
  
}HTTP_input_type_t;

extern ES_WIFI_STAT w_stat2;


void WiFi_test();
void WiFi_stop(const char * msg);
void WiFi_startAP();

void WiFi_begin(void);
bool ConnectWiFi(void);


void HTTP_begin(void);
void HTTP_handleRoot(void);
void HTTP_handleConfig(void);
void HTTP_handleDefault(void);
void HTTP_handleReboot(void);
void HTTP_handleLogo(void);
void HTTP_handlePngStat1(void);
void HTTP_handlePngStat2(void);
void HTTP_handlePngStat3(void);
void HTTP_handlePngWiFi1(void);
void HTTP_handlePngWiFi2(void);
void HTTP_printCSS(String &out);
bool HTTP_login(String &out);
void HTTP_printConfig(String &out);
bool HTTP_checkArgs();


void HTTP_loop();
void WiFi_begin(void);
void Time2Str(char *s,time_t t);
bool SetParamHTTP();
int  HTTP_isAuth();
int  HTTP_checkAuth(const char *pass);
//int  HTTP_checkUserAndPassword( const char *pass);
void HTTP_printInput1(String &out,const char *label, const char *name, const char *value, int size, int len, HTTP_input_type_t htype=HT_TEXT, const char *style=NULL, const char *add_text=NULL);

void HTTP_printHeader(String &out,const char *title, uint16_t refresh=0);
void HTTP_printTail(String &out);
void WiFi_ScanNetwork();
void HTTP_printNetworks1(String &out, const char *name);
void HTTP_goto(const char *url, uint32_t tm, const char *msg);

bool HTTP_redirect();



#endif
