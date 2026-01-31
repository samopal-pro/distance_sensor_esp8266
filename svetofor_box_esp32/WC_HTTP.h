/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/

#ifndef WS_HTTP_h
#define WS_HTTP_h
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "MyConfig.h"
#include "WC_Config.h"
//#include "WC_Led.h"
#include "WC_Task.h"


#define TM_AP_MODE      120000
#define TM_WIFI_CONNECT 20000

extern WebServer server;
extern char SensorID[];
extern bool isAP, isSTA;
extern float Temp, LastTemp, Hum, LastHum, LastDistance, Distance;
extern unsigned long Time;
extern uint32_t msAP, msSTA, msLoad;
extern bool isChangeConfig;
extern SENSOR_STAT_t lastSensorOn;

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
extern bool is_load_page;
extern bool isWiFiAlways1;
//void WiFi_test();
//void WiFi_stop(const char * msg);
//void WiFi_startAP();

void WiFi_begin(void);
//bool ConnectWiFi(void);


void HTTP_begin(void);
void HTTP_handleRoot(void);
void HTTP_handleDistance(void);
void HTTP_handleConfig1(void);
void HTTP_handleConfig2(void);
void HTTP_handleConfig3(void);
void HTTP_handleConfig4(void);
void HTTP_handleDefault(void);
void HTTP_handleReboot(void);
void HTTP_handleUpload(void);
void HTTP_handleLogo(void);
void HTTP_handlePngStat1(void);
void HTTP_handlePngStat2(void);
void HTTP_handlePngStat3(void);
void HTTP_handlePngWiFi1(void);
void HTTP_handlePngWiFi2(void);
void HTTP_handlePngType1(void);
void HTTP_handlePngType2(void);
void HTTP_handlePngType3(void);
void HTTP_handlePngRelay0(void);
void HTTP_handlePngRelay1(void);
void HTTP_handlePngRelay2(void);
void HTTP_handlePngRelay3(void);
void HTTP_handlePngRelay4(void);
void HTTP_handlePngRelay5(void);
void HTTP_handlePlayMP3(void);

void HTTP_handleUpdate(void);
void HTTP_fileUpload1(void);

void HTTP_printCSS(String &out);
void HTTP_printJS(String &out);
bool HTTP_login(String &out);
void HTTP_printConfigRelay(String &out);
void HTTP_printConfigNet(String &out);
void HTTP_printConfig(String &out);
void HTTP_printConfig2(String &out);
void HTTP_printConfig4(String &out);
void HTTP_printConfigColor(String &out);
bool HTTP_checkArgs(int current);
void HTTP_printBottomMenu(String &out);

void HTTP_loop();
void WiFi_begin(void);
void Time2Str(char *s,time_t t);
bool SetParamHTTP();
int  HTTP_isAuth();
int  HTTP_checkAuth(const char *pass);
//int  HTTP_checkUserAndPassword( const char *pass);
void HTTP_printInput1(String &out,const char *label, const char *name, const char *value, int size, int len, HTTP_input_type_t htype=HT_TEXT, const char *style=NULL, const char *add_text=NULL);
void HTTP_InputFloat(String &out,const char *label,const char *name,double value,double min,double max,int size=32,const char *style = NULL);
void HTTP_InputInt(String &out,const char *label,const char *name,int value,int min,int max,int size=32,const char *style = NULL);
void HTTP_InputRange(String &out,const char *label,const char *name,int value,int min,int max,int size=32,const char *style = NULL);
void HTTP_InputInt1(String &out,const char *name,int value,int min,int max,int size=32);
void HTTP_printMessage(const char *s);

void HTTP_InputHidden(String &out, char *name, char *value = "1");


void HTTP_print_color3(String &out, uint32_t color, char *name, char *label, uint32_t color1, uint32_t color2, char *name1 = NULL, bool check = false);
void HTTP_print_color(String &out, uint32_t color, char *name);
void HTTP_print_MP3_7(String &out, char *text, char *name);
void HTTP_print_MP3(String &out, char *text,int dir, int num, int _delay = -1);
void HTTP_checkArgsMP3(char *name);

void HTTP_print_td_color(String &out, uint32_t color, char *name, uint8_t value, uint32_t color_set, uint8_t color_num, uint8_t proc, bool is_change = false);
void HTTP_print_input_radio(String &out,char *name, char *value,bool checked);
void HTTP_print_input_checkbox(String &out,char *name, char *value,bool checked);
void HTTP_print_img_radio(String &out,char *img, char *label, char *name, char *value,bool checked, bool is_table);
void HTTP_print_menu(String &out, int current);
void HTTP_print_menu_item(String &out, bool _isCur, int _proc, char *_label, char *_url, bool _isMP3, int _dirMP3, int _numMP3);
char *HTTP_url( int current );

bool HTTP_redirect();
void HTTP_printHeader(String &out,const char *title, uint16_t refresh=0);
void HTTP_printTail(String &out);
void WiFi_ScanNetwork();
void HTTP_printNetworks1(String &out, const char *name);
void HTTP_goto(const char *url, uint32_t tm, const char *msg);

uint32_t HTMLtoInt(const char *s_color);

bool HTTP_redirect();



#endif
