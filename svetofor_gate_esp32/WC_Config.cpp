 #include "WC_Config.h"

JsonDocument jsonConfig;
JsonDocument jsonNodeList;
JsonDocument jsonGatewayList;
//JsonDocument jsonSave;
//JsonDocument jsonTB;

bool isChangeConfig = true;

//configPID_t configPID;

void configInit(){
   SPIFFS.begin(true);
   listDir("/");   
}


/****************************************************************************************************************************/
// Работа с параметрами настраиваемыми через HTTP 
/****************************************************************************************************************************/
void configPrint(char *label){
   String s;
   serializeJson(jsonConfig, s);   
   Serial.print(F("!!! Config "));
   Serial.print(label);
   Serial.print(" ");
   Serial.println(s);
}


void configRead(){
   jsonConfig.clear();
   File f = SPIFFS.open(CONFIG_JSON, FILE_READ);
   bool save_flag = false;
   if( f ){
      deserializeJson(jsonConfig, f);
      configPrint("Read");
      f.close();
      if( jsonConfig.isNull()     ) save_flag = true; 
   }
   if( !save_flag ){
      if( jsonConfig["CONFIG_V"].isNull() )save_flag = true;
      else if( strcmp(jsonConfig["CONFIG_V"].as<const char *>(),CONFIG_V)!=0 )save_flag = true;
   }
   if( save_flag ){
       configDefault();
       configSave();
   }
}

void configDefault(){
   jsonConfig.clear();
   int n = 0;
// Системные параметры
   jsonConfig["CONFIG_V"]              = CONFIG_V;                //Имя точки доступа устройства  
   jsonConfig["SYSTEM"]["NAME"]        = DEVICE_NAME;                //Имя точки доступа устройства  
   jsonConfig["SYSTEM"]["AP_START"]    = AP_ALWAYS;  
   jsonConfig["SYSTEM"]["PASS0"]       = DEVICE_PASS0;               //Пароль суперадминистратора
   jsonConfig["SYSTEM"]["PASS1"]       = DEVICE_PASS1;               //Пароль администратора
   jsonConfig["SYSTEM"]["PASSS"]       = DEVICE_PASSS;               //Пароль светофорбока (мегоадминистратора)
// Параметры моединения WiFi
   jsonConfig["WIFI"]["NAME"]              = "ASUS_58_2G";                         //Имя сети WiFi
   jsonConfig["WIFI"]["PASS"]              = "sav59vas";                         //Пароль сети WiFi
   jsonConfig["WIFI"]["DHCP"]              = false;                       //Брать парметры сети по DHCP
   jsonConfig["WIFI"]["IP"]["ADDR"]        = "192.168.1.35";             //IP адрес при стаическом режиме
   jsonConfig["WIFI"]["IP"]["MASK"]        = "255.255.255.0";            //IP маска при стаическом режиме
   jsonConfig["WIFI"]["IP"]["GW"]          = "192.168.1.1";              //IP адрес шлюза при стаическом режиме
   jsonConfig["WIFI"]["IP"]["DNS"]         = "8.8.8.8";                  //IP адрес DNS при стаическом режиме
// Параметры подключения к CRM.MOSCOW
//   jsonConfig["NET"]["ENABLE"]            = false;                      //Коннектится по WiFi

   jsonConfig["NET"]["DOGOVOR_ID"]  = "0000";            
   jsonConfig["NET"]["BOX_ID"]      = "1";                     
   jsonConfig["NET"]["T_SEND"]      = 1000;                   //Таймаут отправки на сервер            
   jsonConfig["NET"]["T_RETRY"]     = 10;                     //Таймаут повторной отправки если статус HTTP не 200            


   jsonConfig["CRM_MOSCOW"]["ENABLE"]      = false;                      //посылать информацию на CRM-MOSCOW
   jsonConfig["CRM_MOSCOW"]["SERVER"]      = "crm.moscow";             
   jsonConfig["CRM_MOSCOW"]["PORT"]        = 8001;

   jsonConfig["TB"]["ENABLE"]              = false;                      //Посылать информацию в ThingsBoard
   jsonConfig["TB"]["SERVER"]              = "109.172.115.70";             
   jsonConfig["TB"]["PORT"]                = 8088;

   jsonConfig["MQTT"]["SERVER"]              = "109.172.115.70";             
   jsonConfig["MQTT"]["PORT"]                = 1883;

   jsonConfig["TB"]["TOKEN"]               = "";

   jsonConfig["LORA"]["ENABLE"]            = false;                      //Посылать информацию по Лоре

   

// Настройка подсветки датчика RGB1      
/*
   jsonConfig["RGB1"]["BRIGHTNESS"]    = 5;                          // Яркость ленты 1-10
   jsonConfig["RGB1"]["FREE"]          = COLOR_FREE_DEFAULT;
   jsonConfig["RGB1"]["FREE_BLINK"]    = COLOR_BLINK_DEFAULT;
   jsonConfig["RGB1"]["IS_FREE_BLINK"] = true;
   jsonConfig["RGB1"]["BUSY"]          = COLOR_BUSY_DEFAULT;
   jsonConfig["RGB1"]["IS_NAN_MODE"]   = true;
*/


}

void configSave(){
   File f = SPIFFS.open(CONFIG_JSON, FILE_WRITE);
   serializeJson(jsonConfig, f);
   f.close();
   isChangeConfig = true;
   configPrint("Save");  
}

/****************************************************************************************************************************/
// Работа с ссо списком нод
/****************************************************************************************************************************/
void nodeListRead(){
   jsonNodeList.clear();
   File f = SPIFFS.open(NODE_LIST_JSON, FILE_READ);
   if( f ){
      deserializeJson(jsonNodeList, f);
      nodeListPrint("Read");
      f.close();
   }
}

void nodeListSave(){
   File f = SPIFFS.open(NODE_LIST_JSON, FILE_WRITE);
   serializeJson(jsonNodeList, f);
   f.close();
   nodeListPrint("Save");  
}

void nodeListPrint(char *label){
   String s;
   serializeJson(jsonNodeList, s);   
   Serial.print(F("!!! Node List "));
   Serial.print(label);
   Serial.print(" ");
   Serial.println(s);
}

/****************************************************************************************************************************/
// Работа с ссо списком шлюзов
/****************************************************************************************************************************/
void gateListRead(){
   jsonGatewayList.clear();
   File f = SPIFFS.open(GATEWAY_LIST_JSON, FILE_READ);
   if( f ){
      deserializeJson(jsonGatewayList, f);
      gateListPrint("Read");
      f.close();
   }
}

void gateListSave(){
   File f = SPIFFS.open(GATEWAY_LIST_JSON, FILE_WRITE);
   serializeJson(jsonGatewayList, f);
   f.close();
   gateListPrint("Save");  
}

void gateListPrint(char *label){
   String s;
   serializeJson(jsonGatewayList, s);   
   Serial.print(F("!!! Gateway List "));
   Serial.print(label);
   Serial.print(" ");
   Serial.println(s);
}


void listDir(const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = SPIFFS.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
} 




