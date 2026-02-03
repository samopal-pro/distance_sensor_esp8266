#ifndef WC_NET_h
#define WC_NET_h
#include <WiFi.h>
//#include <RadioLib.h>

#include <HTTPClient.h>


#include "MyConfig.h"
#include "WC_Config.h"
#include "WC_Task.h"


extern char SensorID[];
extern bool isAP, isSTA;


//void taskLora(void *pvParameters);
void taskNet(void *pvParameters);
void handleEventWiFi(arduino_event_id_t event, arduino_event_info_t info);
bool sendHttpParam();

bool sendParamTB();
bool sendAttributeTB();
bool authTB(const char *_key, const char *_secret);

uint16_t KeyGen(char *str);





#endif
