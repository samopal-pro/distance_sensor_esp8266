#include "WC_MQTT.h"

bool isMqttConnect  = false;
bool isMqttAuth     = false;


MQTT_STAT mqttState = MS_NONE;
uint32_t msMqtt     = 0;

WiFiClient     wifiMqttTCP;
PubSubClient   *mqttClient;
#if defined(IS_NTP)
WiFiUDP        wifiNtpUDP;
NTPClient      *ntpClient;
#endif

void mqttInit(){
    mqttClient = new PubSubClient(wifiMqttTCP);
    mqttState = MS_DISCONNECT;
#if defined(IS_NTP)    
    ntpClient  = new NTPClient(wifiNtpUDP);
    ntpClient->begin();
#endif    
}

void mqttLoop(){
   uint32_t ms = millis();
   bool isMqttProvisin = false;
   const char *_user = jsonConfig["TB"]["TOKEN"].as<const char *>();
   if( _user == nullptr || _user[0] == '\0' )isMqttProvisin = true;
   switch( mqttState ){
// Нет соединения, инициируем коннесе      
       case MS_DISCONNECT :
          mqttClient->setServer(jsonConfig["MQTT"]["SERVER"].as<const char *>(), jsonConfig["MQTT"]["PORT"].as<int>()); 
          mqttClient->setKeepAlive(1200);
          if( isMqttProvisin )_user = TB_PROVISION_USER;
          mqttClient->connect(strID,_user,"");
          Serial.printf("!!! MQTT connect %s %d %s\n",jsonConfig["MQTT"]["SERVER"].as<const char *>(), jsonConfig["MQTT"]["PORT"].as<int>(),_user);
          msMqtt = ms;
          mqttState = MS_WAIT_CONNECT;
          break;
// Ждем соединения с интернет
       case MS_WAIT_CONNECT :    
// Если соединение установлено         
          if( mqttClient->connected() ){
               if( isMqttProvisin ){
                  mqttClient->setCallback( mqttCallback );
                  mqttClient->subscribe(PROVISION_RESPONSE_TOPIC);
                  jsonData.clear();
                  jsonData["provisionDeviceKey"]    = TB_PROVISION_KEY;
                  jsonData["provisionDeviceSecret"] = TB_PROVISION_SECRET;
                  jsonData["credentialsType"]       = "ACCESS_TOKEN";
                  jsonData["token"]                 = "";
                  jsonData["deviceName"]            = strID;
                  String s;
                  serializeJson(jsonData, s);
                  Serial.printf("!!! MQTT publish %s %s\n",PROVISION_REQUEST_TOPIC,s.c_str());        
                  mqttClient->publish(PROVISION_REQUEST_TOPIC,s.c_str(), 2);
               }
               else {
                  Serial.println(F("!!! MQTT Subscribe All"));
                  mqttClient->setCallback( mqttCallback );
                  mqttClient->subscribe(TOPIC_DEVICE_ATTRIBUTES);
                  mqttClient->subscribe(TOPIC_GATE_ATTRIBUTES); 
                  mqttClient->subscribe(TOPIC_GATE_ATTRIBUTES_RESPONSE);
                  mqttClient->subscribe(TOPIC_GATE_RPC);               
               }            
               mqttState = MS_CONNECT;
               msMqtt    = ms;
          }
          else {
             if( msMqtt > ms || (ms-msMqtt >= MQTT_WAIT_TM) ){
                Serial.printf("!!! MQTT connect Timeout %d\n",ms-msMqtt); 
                mqttClient->disconnect();
                mqttState = MS_DISCONNECT;
             }   
          }
          break;
       case MS_CONNECT :    
          if( mqttClient->connected() ){
             mqttClient->loop();
          }
          else {
             Serial.println(F("??? MQTT Disconnect"));
             mqttState = MS_DISCONNECT;
          }
          break;

   }
}


void mqttCallback(char* topic, byte* payload, unsigned int len) {
   Serial.print(F("!!! MQTT receive: "));
   Serial.print(topic);
   Serial.print(" ");
   Serial.write(payload, len);
   Serial.println();
   if( strcmp(PROVISION_RESPONSE_TOPIC, topic ) == 0){
      jsonData.clear();
      if( deserializeJson(jsonData, payload, len) ){
         Serial.println(F("??? MQTT Error JSON format"));
         return;
      }
      if( jsonData.containsKey("credentialsValue")  && !jsonData["credentialsValue"].isNull() ){
          jsonConfig["TB"]["TOKEN"] = jsonData["credentialsValue"].as<const char *>();
          configSave();
          mqttClient->disconnect();
          mqttState = MS_DISCONNECT;
          Serial.println(F("!!! MQTT Token OK. Switch to base mode"));
      }
      else {
          Serial.println(F("??? MQTT Token Fail"));

      }
  }
}


void mqttSend(char *_topic, String _payload){
   if( mqttClient->connected() ){
       mqttClient->publish(_topic,_payload.c_str());
       Serial.printf("!!! MQTT %s %s\n",_topic,_payload.c_str() );
   }
}