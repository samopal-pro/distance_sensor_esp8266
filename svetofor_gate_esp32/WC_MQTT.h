#ifndef WC_MQTT_h
#define WC_MQTT_h
#include "WC_Config.h"
#include "WC_Task.h"
#include "src/MQTT/PubSubClient.h" //https://github.com/knolleary/pubsubclient
#include "src/NTPClient/NTPClient.h" 

#define TOPIC_GATE_TELEMETRY "v1/gateway/telemetry"
#define TOPIC_GATE_CONNECT "v1/gateway/connect"
#define TOPIC_GATE_DISCONNECT "v1/gateway/disconnect"
#define TOPIC_GATE_ATTRIBUTES "v1/gateway/attributes"
#define TOPIC_GATE_ATTRIBUTES_REQUEST "v1/gateway/attributes/request"
#define TOPIC_GATE_ATTRIBUTES_RESPONSE "v1/gateway/attributes/response"
#define TOPIC_GATE_REQUEST "v1/gateway/request"
#define TOPIC_GATE_RESPONSE "v1/gateway/response"
#define TOPIC_GATE_RPC "v1/gateway/rpc"
#define TOPIC_GATE_CLAIM "v1/gateway/claim"
#define TOPIC_DEVICE_ATTRIBUTES "v1/devices/me/attributes"
#define TOPIC_DEVICE_ATTRIBUTES_REQUEST "v1/devices/me/attributes/request"
#define TOPIC_DEVICE_ATTRIBUTES_RESPONSE "v1/devices/me/attributes/response"
#define TOPIC_DEVICE_ATTRIBUTES_PLUS "v1/devices/me/attributes/response/+"
#define TOPIC_DEVICE_TELEMETRY "v1/devices/me/telemetry"
#define TOPIC_DEVICE_RPC "v1/devices/me/rpc"
#define TOPIC_FW_RESPONSE_PLUS "v2/fw/response/#"
#define TOPIC_FW_RESPONSE "v2/fw/response"
// Топики для саморегистрации
#define PROVISION_REQUEST_TOPIC "/provision/request"
#define PROVISION_RESPONSE_TOPIC "/provision/response"

enum MQTT_STAT {
  MS_NONE         = -1,
  MS_DISCONNECT   = 0,
  MS_WAIT_CONNECT = 2,
  MS_CONNECT      = 200
};

extern JsonDocument jsonData;
extern MQTT_STAT mqttState;

void mqttInit();
void mqttCallback(char *topic, byte *payload, unsigned int len);
void mqttLoop();
void mqttSend(char *_topic, String _payload);

#endif
