#include "WC_Net.h"

HTTPClient httpClient;
bool isWiFiAlways1 = true; 
uint32_t msSendHttp = 0, msSendTB = 0, msSendLora = 0;
bool isSendAttributeTB = false;
bool isLora            = false;
char curNode[20];
int countFree = -1, countFreeSave = -1;
uint32_t msIR = 0;
IRsend irsend1(PIN_IR);

JsonDocument jsonData;
JsonDocument jsonNodes;
volatile bool loraIrq = false;
//TaskHandle_t loraTaskHandle = NULL;
#ifdef IS_LORA
SPIClass LoraSPI(HSPI);
SX1262    radio = new Module(PIN_LORA_CS, PIN_LORA_DIO1, PIN_LORA_RST, PIN_LORA_BUSY, LoraSPI);
//MyLoRaBaseClass myLora(chipID);
MyLoRaBaseClass myLora((uint64_t)0);
#endif
/**
 * Задача работы с MP3
 * @param pvParameters
 */
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

 void IRAM_ATTR onLoraIrq() {
   loraIrq = true;
   detachInterrupt(PIN_LORA_DIO1);
}

void initLora(){
#ifdef IS_LORA 
   pinMode(PIN_LORA_DIO1, INPUT);

   Serial.print(F("!!! LoRa init "));   
   LoraSPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI);
   int state = radio.begin();
   jsonNodes.clear();
   JsonObject root = jsonNodeList.as<JsonObject>();
   uint32_t _ms = millis();
   for (JsonPair kv : root) {
      if (kv.value().is<JsonObject>()) {
         const char *kv_str = kv.key().c_str();
         JsonObject obj = kv.value().as<JsonObject>();
         jsonNodes[kv_str]["SN"]    = obj["SN"];
         if( obj["State"].isNull())jsonNodes[kv_str]["State"] = -2;
         else jsonNodes[kv_str]["State"] = obj["State"];      
      }
   }
   String s;
   serializeJson(jsonNodes,s);
   Serial.println(s);
   countNodes();
   if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("success!"));
      isLora = true;
      radio.setFrequency(868.0);
      radio.setSpreadingFactor(7);
      radio.setBandwidth(125.0);
      radio.setCodingRate(5);
      attachInterrupt(PIN_LORA_DIO1, onLoraIrq, RISING);
      MyLoRaAddress::Set(myLora.Addr,chipID);
      sendLoraCmd("FFFFFFFFFFFF","test");
      setLoraReceive(true);
    }
   else {
      isLora = false;
      Serial.print(F("failed, code "));
      Serial.println(state);
   }

#endif
}

void readLora(){
#ifdef IS_LORA   
   if( !isLora )return;
   String s;
   uint8_t _buf[MAX_LEN_PAYLOAD];
   setLoraReceive(false);
   int n = radio.getPacketLength();
   if( n > MAX_LEN_PAYLOAD )n= MAX_LEN_PAYLOAD;
   int state = radio.readData(_buf,n);
   int _rssi = radio.getRSSI();
   if (state == RADIOLIB_ERR_NONE) {
      myLora.RX(_buf, n, _rssi);
      if( myLora.StateRX == NSRX_OK || myLora.StateRX == NSRX_BROADCAST ){
         myLora.PrintRX_V3();
         MyLoRaAddress::Get(curNode,myLora.HeaderRX_V3.AddrTX);

//         serializeJson(myLora.Json,s);
//         Serial.printf("!!! LORA RX %d: ",n);
//         Serial.println(s);
         switch( myLora.HeaderRX_V3.Type&B00001111 ){
// Парсим и печатаем входящий пакет
            case PACKET_V3_TYPE_JSON_TELEMETRY:
               sendLoraToMQTT();
               break;
            case PACKET_V3_TYPE_JSON_ATTRIBUTE:
               sendLoraAttrToMQTT();
               break;
         }
         
       }
   } 
  
   setLoraReceive(true);
#endif
}


void setLoraReceive(bool _flag){
 //   loraIrq = false;
    if( _flag ){
       attachInterrupt(PIN_LORA_DIO1, onLoraIrq, RISING);
       radio.startReceive();          
    }
    else {
       radio.standby();
       detachInterrupt(PIN_LORA_DIO1);
     }
}

bool sendLoraAck(){
   myLora.SetHeaderTX(PACKET_V3_TYPE_ACK,myLora.HeaderRX_V3.AddrTX);
   myLora.HeaderTX_V3.CRC = myLora.SetCRC_V3(myLora.BufferTX,myLora.LengthTX);
   int state = radio.transmit(myLora.BufferTX,myLora.LengthTX);
   myLora.PrintTX_V3();
   return (bool)state;  
}

void setCurNode(){
   setCurNode(curNode,"LoRa", myLora.Json["SN"].as<const char *>(),myLora.Json["State"].as<int>(),myLora.Rssi);
}

void setCurNode(const char *_id, const char *_type, const char *_sn, int _state, int _rssi, const char *_ip){
    String s;
   jsonNodes[_id]["Type"]  = _type;
   jsonNodes[_id]["Rssi"]  = _rssi;
   jsonNodes[_id]["Time"]  = millis();
   jsonNodes[_id]["SN"]    = _sn;
   jsonNodes[_id]["State"] = _state;
   if( strcmp(_type,"HTTP") == 0 )jsonNodes[_id]["IP"] = _ip;
   if(!jsonNodeList[_id].isNull()){
      if( jsonNodeList[_id]["State"] != myLora.Json["State"] ){
         jsonNodeList[_id]["State"] = myLora.Json["State"];
         nodeListSave();
      }  
   }
   serializeJson(jsonNodes,s);
   Serial.println(s);
   countNodes();        
}

void countNodes(){
   countFree = 0;
   JsonObject root = jsonNodeList.as<JsonObject>();
   for (JsonPair kv : root) {
      if (kv.value().is<JsonObject>()) {
//         const char *kv_str = kv.key().c_str();
         JsonObject obj = kv.value().as<JsonObject>();
         if( obj["State"].as<int>() != 1 )countFree++;
      }
   }
   Serial.printf("!!! Count free %d %d\n", countFree, countFreeSave);

}

bool sendHttpToMqtt(const char *_id, const char *_sn, const char *_dn, const char *_bn, int _dist, int _stat, uint32_t _uptime, int _rssi, const char *_ip ){
   setCurNode(_id, "HTTP", _sn, _stat, _rssi, _ip);
   String s;
   if( mqttState == MS_CONNECT ){
      jsonData.clear();   
      jsonData[_id][0]["Distance"] = _dist;
      jsonData[_id][0]["State"]    = _stat;
      if( _sn!=NULL && _sn[0]!='\0' )jsonData[_id][0]["SN"] = _sn;
      jsonData[_id][0]["Uptime"]   = _uptime;
      jsonData[_id][0]["Rssi"]     = _rssi;
      jsonData[_id][0]["DN"]       = _dn;
      jsonData[_id][0]["BN"]       = _bn;
      serializeJson(jsonData,s);
      sendLoraAck();
      mqttSend(TOPIC_GATE_TELEMETRY,s);
//      Serial.printf("!!! MQTT send\n");
//      Serial.println(s);
      return true;
   }
   Serial.printf("??? MQTT status %d\n",mqttState);
   return false;
}


bool sendLoraToMQTT(){
   String s;
   myLora.GetJson();
   setCurNode();
   if( mqttState == MS_CONNECT ){
      jsonData.clear();
//      jsonData[_node][0]["values"].set(myLora.Json.as<JsonObject>());
//      jsonData[_node][0]["values"]["RSSI"] = myLora.Rssi;
      jsonData[curNode][0].set(myLora.Json.as<JsonObject>());
      jsonData[curNode][0]["Rssi"] = myLora.Rssi;

      serializeJson(jsonData,s);
      sendLoraAck();
      mqttSend(TOPIC_GATE_TELEMETRY,s);
//      Serial.printf("!!! MQTT send\n");
//      Serial.println(s);
      return true;
   }
   Serial.printf("??? MQTT status %d\n",mqttState);
   return false;
}

bool sendLoraAttrToMQTT(){
   String s;
   myLora.GetJson();
   jsonNodes[curNode]["SerialNo"]  = myLora.Json["SerialNo"];
   jsonNodes[curNode]["DogovorNo"] = myLora.Json["DogovorNo"];
   jsonNodes[curNode]["BoxNo"]     = myLora.Json["BoxNo"];
   serializeJson(jsonNodes,s);
   Serial.println(s);
   if( mqttState == MS_CONNECT ){
      jsonData.clear();
      jsonData[curNode].set(myLora.Json.as<JsonObject>());


      jsonData[curNode]["GetewayID"] = strID;
      serializeJson(jsonData,s);
      sendLoraAck();


      mqttSend(TOPIC_GATE_ATTRIBUTES,s);
      return true;
   }
   Serial.printf("??? MQTT status %d\n",mqttState);
   return false;
}



bool sendLoraCmd(const char *_node, const char *_cmd){
   int state = 0;
#ifdef IS_LORA   
   if( !isLora )return false;
   uint64_t _node64 = strtoull((char *)_node, NULL, 16);
   uint8_t _node_addr[6];
   MyLoRaAddress::Set(_node_addr, _node64);
   myLora.SetHeaderTX(PACKET_V3_TYPE_JSON_RPC,_node_addr);
   myLora.Json["RPC"] = _cmd;
   myLora.SetJsonBodyTX();
   state = radio.transmit(myLora.BufferTX,myLora.LengthTX);
   myLora.PrintTX_V3();
#endif
   return (bool)state;  

}


/**
 * Задача передача данных через WiFi
 * @param pvParameters
 */
void taskNet( void *pvParameters ){
#if defined(DEBUG_SERIAL)
    Serial.println(F("!!! WiFi task start"));
#endif
   WiFi.mode(WIFI_OFF);
   uint32_t ms2 = 0, ms3 = 0;
   EventRGB1->setColor0(COLOR_BLACK);
   EventRGB1->setColor1(COLOR_BLACK);
   Network.onEvent(handleEventWiFi);
   if( jsonConfig["SYSTEM"]["AP_START"].as<bool>() ||  bootCount<1 )isAP = true;
   else isAP = false;
   isSTA = false;
   initLora();
   mqttInit();
   InitIR();
//   HTTP_begin();
   while(true){
      if(isSensorBlock || calibrMode == CM_WAIT_REBOOT ){ //Сенсор заблокирован до выключения питания
         vTaskDelay(1000);    
         continue;
      }

      uint32_t ms = millis();
      if( ms2 == 0 || ms < ms2 || (ms-ms2)>3000){
          ms2 = ms;
          wifi_mode_t curWiFi = WiFi.getMode(); 
          if( !jsonConfig["WIFI"]["NAME"].isNull() )isSTA = true;
          else isSTA = false;
// Стартуем точку доступа          
          if( isAP && ( curWiFi != WIFI_AP && curWiFi != WIFI_AP_STA) ){
             WiFi_ScanNetwork();
             WiFi.enableAP(true);
             WiFi.softAP(jsonConfig["SYSTEM"]["NAME"].as<String>());
             HTTP_begin();
             msAP = millis();                
          }
// Гасим точку доступа
          if( !isAP && ( curWiFi == WIFI_AP || curWiFi == WIFI_AP_STA) ){
//             Serial.println(F("!!! Disable AP"));
             WiFi.enableAP(false);
          }
          if( isSTA && ( curWiFi != WIFI_STA && curWiFi != WIFI_AP_STA) ){
             Serial.println(F("!!! Start STA"));
             msSTA = ms;
             WiFi_ScanNetwork();
             WiFi.enableSTA(true);
             if( jsonConfig["WIFI"]["DHCP"].as<bool>() == false ){
                IPAddress ip_addr,ip_mask,ip_gate,ip_dns;
                if( ip_addr.fromString( jsonConfig["WIFI"]["IP"]["ADDR"].as<String>() ) &&
                    ip_mask.fromString( jsonConfig["WIFI"]["IP"]["MASK"].as<String>() ) &&
                    ip_gate.fromString( jsonConfig["WIFI"]["IP"]["GW"].as<String>() ) &&
                    ip_dns.fromString(  jsonConfig["WIFI"]["IP"]["DNS"].as<String>()     ) ){
                   Serial.println("Config static IP address");
                   WiFi.config(ip_addr,ip_gate,ip_mask,ip_dns);
                }                 
             }
             WiFi.begin(jsonConfig["WIFI"]["NAME"].as<String>(), jsonConfig["WIFI"]["PASS"].as<String>());
//             ledSTA(true);
             EventRGB1->setColor1(COLOR_WIFI_WAIT);
          }      
          if( isSTA && ( curWiFi == WIFI_STA || curWiFi == WIFI_AP_STA) ){
             if( WiFi.status() != WL_CONNECTED && (ms - msSTA)>10000 ){
                 Serial.println(F("!!! Error WiFi"));
                 EventRGB1->setColor1(COLOR_WIFI_OFF);
                 WiFi.enableSTA(false);
             }
          }
          if( !isSTA && ( curWiFi == WIFI_STA || curWiFi == WIFI_AP_STA) ){
             WiFi.enableSTA(false);
//             ledSTA( false );
             EventRGB1->setColor1(COLOR_BLACK);
          }

// Стартуем подключение

      }
      if( (ms3 == 0 || ms < ms3 || (ms-ms3)>2000) ){
          ms3 = ms;
      }
      if(WiFi.status() == WL_CONNECTED)mqttLoop();
      if( loraIrq ){
         loraIrq = false;
         readLora();
      }
      HTTP_loop();
      if( countFree != countFreeSave ){
          if( SendIR(countFree))countFreeSave = countFree;
      }
      
      vTaskDelay(50);      
   }


}

void handleEventWiFi(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:     Serial.println("STA Started"); break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED: Serial.println("STA Connected"); break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      EventRGB1->setColor1(COLOR_WIFI_ON);
      Serial.println("!!! STA Got IP");
      Serial.println(WiFi.STA);
//      WiFi.AP.enableNAPT(true);
      break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      Serial.println("!!! STA Lost IP");
      EventRGB1->setColor1(COLOR_WIFI_WAIT);          
      msSTA = millis();
//      WiFi.AP.enableNAPT(false);
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("!!! STA Disconnected");
//      WiFi.AP.enableNAPT(false);
      break;
    case ARDUINO_EVENT_WIFI_STA_STOP: Serial.println("STA Stopped"); break;

    case ARDUINO_EVENT_WIFI_AP_START:
//      w_stat2 = EWS_AP_MODE;
      if( bootCount<0 )   EventRGB1->setColor0(COLOR_WIFI_AP);
      else EventRGB1->setColor0(COLOR_WIFI_AP1);
      Serial.println("AP Started");
      Serial.println(WiFi.AP);
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:    
      Serial.println("AP STA Connected");
      EventRGB1->setColor0(COLOR_WIFI_ON,true); 
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
     Serial.println("AP STA Disconnected"); 
     EventRGB1->setColor0(COLOR_WIFI_AP1);
     break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      Serial.print("AP STA IP Assigned: ");
      Serial.println(IPAddress(info.wifi_ap_staipassigned.ip.addr));
      break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED: Serial.println("AP Probe Request Received"); break;
    case ARDUINO_EVENT_WIFI_AP_STOP:       
      Serial.println("AP Stopped"); 
      EventRGB1->setColor0(COLOR_BLACK);
      break;

    default: break;
  }
}

bool sendHttpParam(){
   bool ret = false;
   char s[64];
   uint32_t tm = millis()/1000;

   sprintf(s,"%s;%ld;%d;%d;%d",strID,tm,(int)Distance,tm,0);
   uint16_t crc = KeyGen(s);

   String str = "";
   str += "http://";
   str += jsonConfig["CRM_MOSCOW"]["SERVER"].as<String>();
   str += ":";
   str += jsonConfig["CRM_MOSCOW"]["PORT"].as<int>();
   str += HTTP_PATH;
   str += "?id=";
   str += jsonConfig["NET"]["DOGOVOR_ID"].as<String>();
   str += "_";
   str += jsonConfig["NET"]["BOX_ID"].as<String>();
//   str += strID;
   str += "&temp=0&hum=0&dist=";
   str += String(Distance,0);
   str += "&tm=";
   str += String(millis()/1000);
   str += "&btn=";
   switch(SensorOn){
      case SS_BUSY:
      case SS_NAN_BUSY: str += "1";break;
      case SS_FREE:   
      case SS_NAN_FREE: str += "0";break;  
      default: str += "-1";
   }   
   str += "&uptime=";
   str += String(millis()/1000);
   str += "&key=";
   str += (int)crc;

//   httpClient.begin(jsonConfig["CRM_MOSCOW"]["SERVER"].as<String>(), jsonConfig["CRM_MOSCOW"]["PORT"].as<int>(),str);
   Serial.println(str);
   httpClient.begin(str);
   int httpCode = httpClient.GET();
   Serial.print(F("!!! HTTP send "));
   Serial.println(jsonConfig["CRM_MOSCOW"]["SERVER"].as<String>());
   
   if( httpCode == HTTP_CODE_OK ){
        String payload = httpClient.getString();
        Serial.print(" success: ");
        Serial.println(payload);
        ret = true;
   }
   else {
        Serial.print(" error: ");
        Serial.println(httpCode);
   }
//   sprintf(sbuf,"GET http://%s:%d%s?id=%s&temp=%d&hum=%d&dist=%d&tm=%ld&btn=%d&uptime=%ld&key=%d HTTP/1.0\r\n\r\n",
//      EA_Config.SERVER,EA_Config.PORT,HTTP_PATH,SensorID,_temp,_hum,
//      _dist,_time,(int)_btn,_uptime,(int)KeyGen());
   httpClient.end();
   return ret;    
}

/**
 * Генерация контрольной суммы 
 */
 
uint16_t KeyGen(char *str){
//   char s[64];
//   sprintf(s,"%s;%ld;%d;%d;%d",strID,Time,Distance,Time,Hum);
//   Serial.printf("!!! CRC %s %d\n",str,strlen(str));
   uint16_t crc = 0;
   for( int i=0; i< strlen(str); i++ ){
       crc += (int)str[i];
   }    
   crc = ( ~ crc )&0xfff;   
   return crc;
  
}

void InitIR(){
   Serial.println(F("!!!! Init IR"));
   irsend1.begin();
//   SendIR(0);

//   xSemaphoreTake( SemaphoreIR, 100 );
}

bool SendIR(uint16_t _num){
  uint32_t ms = millis();
   if( ms - msIR < 2000 )return false;
   msIR = ms;
//   if( _num == saveNum )return;
   SetSemaphoreIR(true);
//   saveNum = _num;
   uint8_t _ir_cmd[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};

   if( _num == 0 ){
      _num = MAX_NUM_PARKING_SPACE+1;
   }
   Serial.printf("!!!! Send IR %d\n",_num);
//   numIR = _num;
  if( _num < 10 ){
       irsend1.sendNEC(irsend1.encodeNEC(0x00, _ir_cmd[_num]),32,0);

//       IrSender.sendNEC(0x00, _ir_cmd[_num], 0);
   }
   else if( _num < 100){
       irsend1.sendNEC(irsend1.encodeNEC(0x00, _ir_cmd[_num/10]),32,0);
       SetSemaphoreIR(false);
       delay(100);
       SetSemaphoreIR(true);

       irsend1.sendNEC(irsend1.encodeNEC(0x00, _ir_cmd[_num%10]),32,0);
   }      
   else  {
       irsend1.sendNEC(irsend1.encodeNEC(0x00, _ir_cmd[_num/100]),32,0);
       SetSemaphoreIR(false);
       delay(100);
       SetSemaphoreIR(true);
       irsend1.sendNEC(irsend1.encodeNEC(0x00, _ir_cmd[(_num/10)%10]),32,0);
       SetSemaphoreIR(false);
       delay(100);
       SetSemaphoreIR(true);
       irsend1.sendNEC(irsend1.encodeNEC(0x00, _ir_cmd[_num%10]),32,0);
     
   }       
   SetSemaphoreIR(false);
   return true;
}


void SetSemaphoreIR(bool flag){
   if( flag ){
//      Serial.println(F("!!!! Wait IR Semaphore"));
      taskENTER_CRITICAL(&mux);
//      xSemaphoreTake( SemaphoreIR, portMAX_DELAY );
//      Serial.println(F("!!!! Set IR Semaphore ON"));  
   }
   else {
//      Serial.println(F("!!!! Set IR Semaphore OFF"));
      taskEXIT_CRITICAL(&mux);
//      xSemaphoreGive( SemaphoreIR ); 
      
   }
}



