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

#define TM_AP_MODE      120000
#define TM_WIFI_CONNECT 20000

extern ESP8266WebServer server;
extern char SensorID[];
extern bool isAP;
extern int           Temp;
extern int           Hum;
extern int           Distance;
extern unsigned long Time;
extern uint32_t msAP,msSTA;

enum ES_WIFI_STAT {
  EWS_OFF,
  EWS_AP_MODE,
  EWS_NOT_CONFIG,
  EWS_WAIT,
  EWS_ON
};

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
void HTTP_loop();
void WiFi_begin(void);
void Time2Str(char *s,time_t t);
bool SetParamHTTP();
int  HTTP_isAuth();
void HTTP_handleLogin(void);
void HTTP_gotoLogin();
int  HTTP_checkAuth(const char *pass);
//int  HTTP_checkUserAndPassword( const char *pass);
void HTTP_printInput(String &out,const char *label, const char *name, const char *value, int size, int len,bool is_pass=false);
void HTTP_printHeader(String &out,const char *title, uint16_t refresh=0);
void HTTP_printTail(String &out);
void WiFi_ScanNetwork();
void HTTP_printNetworks(String &out);




#endif
