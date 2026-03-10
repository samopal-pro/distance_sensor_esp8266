#ifndef WC_NET_h
#define WC_NET_h
#include "MyConfig.h"

#ifdef IS_LORA
#include <SPI.h>
#include <RadioLib.h>
#include "src/MyLoRa/MyLoRaBase.h"
#endif

#include <WiFi.h>
#include <HTTPClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#include "WC_Config.h"
#include "WC_Task.h"
#include "WC_MQTT.h"

extern char SensorID[];
extern bool isAP, isSTA;
extern bool isLora;

void taskLora(void *pvParameters);
void initLora();
void readLora();
bool sendLora();
bool sendLoraAck();
bool sendLoraCmd(const char *_node, const char *_cmd);
bool sendLoraToMQTT();
bool sendLoraAttrToMQTT();
void setLoraReceive(bool _flag);
bool sendHttpToMqtt(const char *_id, const char *_sn, const char *_dn, const char *_bn, int _dist, int _stat, uint32_t _uptime, int _rssi, const char *_ip );
void setCurNode(const char *_id, const char *_type, const char *_sn, int _state, int _rssi, const char *_ip = NULL);
void setCurNode();


void countNodes();

void InitIR();
bool SendIR(uint16_t _num);
void SetSemaphoreIR(bool flag);

void IRAM_ATTR onLoraIrq();
void taskNet(void *pvParameters);
void handleEventWiFi(arduino_event_id_t event, arduino_event_info_t info);
bool sendHttpParam();

bool sendParamTB();
bool sendAttributeTB();
bool authTB(const char *_key, const char *_secret);

uint16_t KeyGen(char *str);





#endif
