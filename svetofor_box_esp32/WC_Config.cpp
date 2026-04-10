#include "WC_Config.h"


JsonDocument jsonConfig;
JsonDocument jsonSave;
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

      T_CONFIG _config = (T_CONFIG)jsonSave["CONFIG"].as<int>();
      configSet(CFG_TEST);
      configDefault();
      configSave();
      configSet(CFG_CARWASH);
      configDefault();
      configSave();
      configSet(CFG_PARKING);
      configDefault();
      configSave();
      configSet(_config);
   }
}

String deviceNmae( char *_name){
    String s = _name;
    if( strlen(serNo) >0 )s.replace("2025",serNo);
    return s;
}


void configSave(){
   Serial.printf("!!! Save %s\n",CONFIG_JSON);
   File f = SPIFFS.open(CONFIG_JSON, FILE_WRITE);
   serializeJson(jsonConfig, f);
   f.close();
   isChangeConfig = true;
   configPrint("Save");  
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


void savePrint(char *label){
   String s;
   serializeJson(jsonSave, s);   
   Serial.print(F("!!! Save "));
   Serial.print(label);
   Serial.print(" ");
   Serial.println(s);
}


void saveRead(){
   jsonSave.clear();
   File f = SPIFFS.open(SAVE_JSON, FILE_READ);
   bool save_flag = false;
   if( f ){
      deserializeJson(jsonSave, f);
      savePrint("Read");
      f.close();
      if( jsonSave.isNull()     ) save_flag = true; 
   }
   if( save_flag ){
       saveDefault();
       saveSave();
   }
   strcpy(CONFIG_JSON, CONFIG_JSON_TEST); 
   if( !jsonSave["CONFIG"] )jsonSave["CONFIG"] = CFG_TEST;
   configSet();
}

void saveDefault(){
   jsonSave.clear();
   jsonSave["DISTANCE"]    = 0;
   jsonSave["STATE_ON"]    = SENSOR_GROUND_STATE;
   jsonSave["BOOT_COUNT"]  = 0;
   jsonSave["MSG_AUTH"]    = true;
   jsonSave["CONFIG"]      = CFG_TEST;
}

void saveSave(){
   File f = SPIFFS.open(SAVE_JSON, FILE_WRITE);
   serializeJson(jsonSave, f);
   f.close();

   savePrint("Save");  
}

void saveSet(float _dist, SENSOR_STAT_t _on){
   jsonSave["DISTANCE"]    = _dist;
   jsonSave["STATE_ON"]    = (int)_on;
   saveSave();
   
}

uint16_t saveCount(){
   uint16_t _count = 0;
   if( jsonSave["BOOT_COUNT"] == "" ){
      jsonSave["BOOT_COUNT"] = _count;   
   }
   else {
      _count = jsonSave["BOOT_COUNT"].as<uint16_t>();
      jsonSave["BOOT_COUNT"] = _count+1;
   }
   saveSave();
   return _count;
}
