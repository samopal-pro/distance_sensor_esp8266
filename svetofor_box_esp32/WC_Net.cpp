#include "WC_Net.h"

HTTPClient httpClient;
bool isWiFiAlways1 = true; 
uint32_t msSendHttp = 0, msSendTB = 0, msSendLora = 0;
bool isSendAttributeTB = false;
bool isLora            = false;



JsonDocument jsonData;
volatile bool loraIrq = false;
//TaskHandle_t loraTaskHandle = NULL;
#ifdef IS_LORA
SPIClass LoraSPI(HSPI);
SX1262    radio = new Module(PIN_LORA_CS, PIN_LORA_DIO1, PIN_LORA_RST, PIN_LORA_BUSY, LoraSPI);
MyLoRaBaseClass myLora(chipID);
#endif
/**
 * Задача работы с MP3
 * @param pvParameters
 */


 void IRAM_ATTR onLoraIrq() {
   loraIrq = true;
   detachInterrupt(PIN_LORA_DIO1);
//  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//  vTaskNotifyGiveFromISR(loraTaskHandle, &xHigherPriorityTaskWoken);
//  if (xHigherPriorityTaskWoken) {
//    portYIELD_FROM_ISR();
//  }
}

void initLora(){
#ifdef IS_LORA 
   pinMode(PIN_LORA_DIO1, INPUT);

   Serial.print(F("!!! LoRa init "));   
   LoraSPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI);
   int state = radio.begin();
   if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("success!"));
      isLora = true;
      radio.setFrequency(868.0);
      radio.setSpreadingFactor(7);
      radio.setBandwidth(125.0);
      radio.setCodingRate(5);
      attachInterrupt(PIN_LORA_DIO1, onLoraIrq, RISING);
      radio.startReceive();
      MyLoRaAddress::Set(myLora.AddrRX,0,0,0,0,0,0);

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
   int state = radio.readData(s);

   if (state == RADIOLIB_ERR_NONE) {
      Serial.print("RX: ");
      Serial.println(s);
   } 
   else {
      Serial.print("RX error: ");
      Serial.println(state);
   }
  
       // снова в приём
   attachInterrupt(PIN_LORA_DIO1, onLoraIrq, RISING);
   radio.startReceive();   
#endif
}

bool sendLora(){
#ifdef IS_LORA

    int _state = -1;
    switch(SensorOn){
       case SS_BUSY:
       case SS_NAN_BUSY: _state = 1;break;
       case SS_FREE:   
       case SS_NAN_FREE: _state = 0;break;  
    }   

    myLora.SetHeaderTX(PACKET_V3_TYPE_JSON_TELEMETRY, myLora.AddrRX, true);
    myLora.Json["Distance"] = (int)Distance;
    myLora.Json["State"]    = _state;
    myLora.Json["Uptime"]   = esp_timer_get_time()/1000000;
    if (myLora.SetJsonBodyTX()) {
       for( int i=0; i<3; i++ ){
          int state = radio.transmit(myLora.BufferTX,myLora.LengthTX);

          if (state == RADIOLIB_ERR_NONE) {
             Serial.println("!!! LORA TX OK");
          }
          else {
             Serial.print("??? LORA TX error: ");
             Serial.println(state);
          }         
       } 
   }
   radio.startReceive();  

#endif
   return true;
}

/*
void taskLora(void *pvParameters) {
#if defined(DEBUG_SERIAL)
   Serial.println(F("!!! LoRa task start"));
#endif
   

   


// прерывание от DIO1
   pinMode(PIN_LORA_DIO1, INPUT);

   SX1262 radio = new Module(PIN_LORA_CS, PIN_LORA_DIO1, PIN_LORA_RST, PIN_LORA_BUSY, LoraSPI);
   
   LoraSPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI);
   int state = radio.begin();
   if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("success!"));
   }
   else {
      Serial.print(F("failed, code "));
      Serial.println(state);
   }
   radio.setFrequency(868.0);
   radio.setSpreadingFactor(7);
   radio.setBandwidth(125.0);
   radio.setCodingRate(5);

   attachInterrupt(PIN_LORA_DIO1, onLoraIrq, RISING);
   radio.startReceive();
   String packet;
   while (true) {
      Serial.println(F("!!! LoRa wait ..."));
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

      int state = radio.readData(packet);

      if (state == RADIOLIB_ERR_NONE) {
         Serial.print("RX: ");
         Serial.println(packet);
      } 
      else {
         Serial.print("RX error: ");
         Serial.println(state);
       }
  
       // снова в приём
       radio.startReceive();
//      vTaskDelay(1000);
   }
}
*/

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
   initLora();
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

/*
          wifi_mode_t modeWiFi =  WIFI_OFF;
          if( isAP && isSTA )modeWiFi = WIFI_AP_STA;
          else if( isAP )modeWiFi     = WIFI_AP;
          else if( isSTA )modeWiFi    = WIFI_STA; 
*/

      }
      if( (ms3 == 0 || ms < ms3 || (ms-ms3)>2000) ){
          ms3 = ms;
          if(WiFi.status() == WL_CONNECTED){
             if(  isSendNet ){
                isSendNet  = false;
                msSendHttp = 0;
                msSendTB   = 0;
                msSendLora = 0;
             } 
             if( jsonConfig["CRM_MOSCOW"]["ENABLE"].as<bool>() && ( msSendHttp == 0 || msSendHttp < ms ) ){
                if( sendHttpParam() )msSendHttp = ms + jsonConfig["NET"]["T_SEND"].as<uint32_t>()*1000;
                else msSendHttp = ms + jsonConfig["NET"]["T_RETRY"].as<uint32_t>()*1000;
             }
             if( jsonConfig["TB"]["ENABLE"].as<bool>() && ( msSendTB == 0 || msSendTB < ms ) ){
                if( sendParamTB() )msSendTB = ms + jsonConfig["NET"]["T_SEND"].as<uint32_t>()*1000;
                else msSendTB = ms + jsonConfig["NET"]["T_RETRY"].as<uint32_t>()*1000;
             }
          }
          if( isLora ){
             if( jsonConfig["LORA"]["ENABLE"].as<bool>() && ( msSendLora == 0 || msSendLora < ms )  ){
                if( sendLora() )msSendLora = ms + jsonConfig["NET"]["T_SEND"].as<uint32_t>()*1000;
                else msSendLora = ms + jsonConfig["NET"]["T_RETRY"].as<uint32_t>()*1000;
             }
          }
          
/*             
             if( sendHttpParam() )ms_tmp = ms + jsonConfig["CRM_MOSCOW"]["T_SEND"].as<uint32_t>()*1000;
             else ms_tmp = ms + jsonConfig["CRM_MOSCOW"]["T_RETRY"].as<uint32_t>()*1000;
             if( msSendHttp == 0)msSendHttp = ms_tmp;
*/
      }
      if( loraIrq ){
         loraIrq = false;
         readLora();
      }
      HTTP_loop();
      
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
   uint16_t crc = 0;
   for( int i=0; i< strlen(str); i++ ){
       crc += (int)str[i];
   }    
   crc = ( ~ crc )&0xfff;   
   return crc;
  
}

bool sendParamTB(){
    bool ret = false;
    if( jsonConfig["TB"]["TOKEN"].isNull() || jsonConfig["TB"]["TOKEN"] == "" )if(!authTB(TB_PROVISION_KEY,TB_PROVISION_SECRET))return false;

    String _url = "http://";
    _url += jsonConfig["TB"]["SERVER"].as<String>();
    _url += ":";
    _url += jsonConfig["TB"]["PORT"].as<int>();
    _url += "/api/v1/";
    _url += jsonConfig["TB"]["TOKEN"].as<String>();
    _url += "/telemetry";

    int _state = -1;
    switch(SensorOn){
       case SS_BUSY:
       case SS_NAN_BUSY: _state = 1;break;
       case SS_FREE:   
       case SS_NAN_FREE: _state = 0;break;  
    }   
    jsonData.clear();
    jsonData["Distance"] = (int)Distance;
    jsonData["State"]    = _state;
    jsonData["Uptime"]   = esp_timer_get_time()/1000000;
    String _data;
    serializeJson(jsonData, _data); 

    httpClient.begin(_url);
    httpClient.addHeader("Content-Type", "application/json");     
    int httpCode = httpClient.POST(_data);
    Serial.print(F("!!! TB auth send "));
    Serial.print(_url);
    Serial.print(" ");
    Serial.println(_data);
   
   if( httpCode == HTTP_CODE_OK ){
        String _payload = httpClient.getString();
        Serial.print(" success");
        Serial.println(_payload);
        ret = true;
   }
   else {
        Serial.print(" error: ");
        Serial.println(httpCode);
   }
   httpClient.end();
   if( !isSendAttributeTB && ret )isSendAttributeTB = sendAttributeTB();
   return ret;    


}

// Работа с TB
// Получаеи JWT_TOKEN
bool authTB(const char *_key, const char *_secret){
    bool ret = false;
    String _url = "http://";
    _url += jsonConfig["TB"]["SERVER"].as<String>();
    _url += ":";
    _url += jsonConfig["TB"]["PORT"].as<int>();
    _url += "/api/v1/provision";

    jsonData.clear();
    jsonData["deviceName"]            = strID;
    jsonData["provisionDeviceKey"]    = _key;
    jsonData["provisionDeviceSecret"] = _secret;
    String _data;
    serializeJson(jsonData, _data); 

    httpClient.begin(_url);
    httpClient.addHeader("Content-Type", "application/json");     
    int httpCode = httpClient.POST(_data);
    Serial.print(F("!!! TB auth send "));
    Serial.print(_url);
    Serial.print(" ");
    Serial.println(_data);

   if( httpCode == HTTP_CODE_OK ){
      String _payload = httpClient.getString();
      jsonData.clear();
      deserializeJson(jsonData,_payload);
      Serial.println(_payload);
      if( jsonData["status"].as<String>() == "SUCCESS" ){
         jsonConfig["TB"]["TOKEN"] =  jsonData["credentialsValue"].as<String>();
         configSave();
         ret = true;  
      }
   }
   return ret;
}


bool sendAttributeTB(){
    bool ret = false;

    String _url = "http://";
    _url += jsonConfig["TB"]["SERVER"].as<String>();
    _url += ":";
    _url += jsonConfig["TB"]["PORT"].as<int>();
    _url += "/api/v1/";
    _url += jsonConfig["TB"]["TOKEN"].as<String>();
    _url += "/attributes?clientKeys";
    jsonData.clear();
    jsonData["SerialNo"]    = serNo;
    jsonData["DogovorNo"]   = jsonConfig["NET"]["DOGOVOR_ID"].as<String>();
    jsonData["BoxNo"]       = jsonConfig["NET"]["BOX_ID"].as<String>();
    String _data;
    serializeJson(jsonData, _data); 

    httpClient.begin(_url);
    httpClient.addHeader("Content-Type", "application/json");     
    int httpCode = httpClient.POST(_data);
    Serial.print(F("!!! TB send token="));
    Serial.print(jsonConfig["TB"]["TOKEN"].as<String>());
    Serial.print(" Data=");
    Serial.println(_data);
   
   if( httpCode == HTTP_CODE_OK ){
        String _payload = httpClient.getString();
        Serial.print(" success");
        Serial.println(_payload);
        ret = true;
   }
   else {
        Serial.print(" error: ");
        Serial.println(httpCode);
   }
   httpClient.end();

   return ret;    
}

