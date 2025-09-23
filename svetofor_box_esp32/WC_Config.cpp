 #include "WC_Config.h"

JsonDocument jsonConfig;
JsonDocument jsonSave;

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
   if( save_flag ){
       configDefault();
       configSave();
   }
}

void configDefault(){
   jsonConfig.clear();
   int n = 0;
// Системные параметры
   jsonConfig["SYSTEM"]["NAME"]        = DEVICE_NAME;                //Имя точки доступа устройства  
   jsonConfig["SYSTEM"]["AP_START"]    = AP_ALWAYS;  
   jsonConfig["SYSTEM"]["PASS0"]       = DEVICE_PASS0;               //Пароль суперадминистратора
   jsonConfig["SYSTEM"]["PASS1"]       = DEVICE_PASS1;               //Пароль администратора
// Параметры моединения WiFi
   jsonConfig["WIFI"]["NAME"]          = "";                         //Имя сети WiFi
   jsonConfig["WIFI"]["PASS"]          = "";                         //Пароль сети WiFi
   jsonConfig["WIFI"]["DHCP"]          = true;                       //Брать парметры сети по DHCP
   jsonConfig["WIFI"]["IP"]["ADDR"]    = "192.168.1.10";             //IP адрес при стаическом режиме
   jsonConfig["WIFI"]["IP"]["MASK"]    = "255.255.255.0";            //IP маска при стаическом режиме
   jsonConfig["WIFI"]["IP"]["GW"]      = "192.168.1.1";              //IP адрес шлюза при стаическом режиме
   jsonConfig["WIFI"]["IP"]["DNS"]     = "8.8.8.8";                  //IP адрес DNS при стаическом режиме
// Параметры подключения к CRM.MOSCOW
   jsonConfig["CRM_MOSCOW"]["ENABLE"]  = false;                      //посылать информацию на CRM-MOSCOW
   jsonConfig["CRM_MOSCOW"]["DOGOVOR_ID"]  = "0000";            
   jsonConfig["CRM_MOSCOW"]["BOX_ID"]      = "1";                     
   jsonConfig["CRM_MOSCOW"]["SERVER"]      = "crm.moscow";             
   jsonConfig["CRM_MOSCOW"]["PORT"]        = 8081;             
   jsonConfig["CRM_MOSCOW"]["T_SEND"]      = 1000;                   //Таймаут отправки на сервер            
   jsonConfig["CRM_MOSCOW"]["T_RETRY"]     = 10;                     //Таймаут повторной отправки если статус HTTP не 200            
   
   jsonConfig["SENSOR"]["TYPE"]        = DEFAULT_SENSOR_TYPE; //Тип установки/срабатывания датчика
   jsonConfig["SENSOR"]["INSTALL"]     = DEFAULT_SENSOR_INSTALL_TYPE; //Тип установки/срабатывания датчика
// Эти значения работают при типе MEASURE_TYPE_NORMAL   
   jsonConfig["SENSOR"]["DIST_GROUND"] = DEFAULT_SENSOR_GROUND;       //Высота установки датчика (расстояние до пола)
   jsonConfig["SENSOR"]["DIST_LIMIT"]  = DEFAULT_SENSOR_LIMIT;        //Если разница между текущим показанием и высатой установки больше этого значения, то датчик показывает занято
//   jsonConfig["SENSOR"]["DIST_ZERO"]   = 11111;                       //Дистанция если меньше лимита
// Эти значения работают при типе  MEASURE_TYPE_OUTSIDE
   jsonConfig["SENSOR"]["DIST_MIN1"]   = DEFAULT_SENSOR_MIN_DIST;     //Минимальная дистанция диапазона
   jsonConfig["SENSOR"]["DIST_MAX1"]   = DEFAULT_SENSOR_MAX_DIST;     //Максимальная дистанция диапазона
// Эти значения работают при типе MEASURE_TYPE_INSIDE
   jsonConfig["SENSOR"]["DIST_MIN2"]   = DEFAULT_SENSOR_MIN_DIST;     //Минимальная дистанция диапазона
   jsonConfig["SENSOR"]["DIST_MAX2"]   = DEFAULT_SENSOR_MAX_DIST;     //Максимальная дистанция диапазона
// Таймаут цикла опроса датчиков
   jsonConfig["SENSOR"]["T_LOOP"]      = 1;
// Настройка цикла калибровки    
   jsonConfig["CALIBR"]["DELAY_START"] = 5;                           //Задержка начала калибровки после старта контроллера
   jsonConfig["CALIBR"]["NUMBER"]      = 10;                          //Количество корректных измерений при калибровке

// Настройка подсветки датчика RGB1      
   jsonConfig["RGB1"]["BRIGHTNESS"]    = 10;                          // Яркость ленты 1-10
   jsonConfig["RGB1"]["FREE"]          = COLOR_FREE_DEFAULT;
   jsonConfig["RGB1"]["FREE_BLINK"]    = COLOR_BLINK_DEFAULT;
   jsonConfig["RGB1"]["IS_FREE_BLINK"] = true;
   jsonConfig["RGB1"]["BUSY"]          = COLOR_BUSY_DEFAULT;
   jsonConfig["RGB1"]["IS_NAN_MODE"]   = true;
   jsonConfig["RGB1"]["NAN_MODE"]      = NAN_VALUE_IGNORE;

// Настройка реле1   
   jsonConfig["RELAY1"]["MODE"]        = RELAY_NORMAL;               // Режим работы реле
   jsonConfig["RELAY1"]["INVERSE"]     = false;                      // Флаг иверсии реле
   jsonConfig["RELAY1"]["T_PULSE"]     = 1;                          // Длительность импульса при импульсном режиме
   jsonConfig["RELAY1"]["T_PAUSE"]     = 1;                          // Длительность паузы при импульсном режиме
   jsonConfig["RELAY1"]["DELAY_ON"]    = 5;                          // Залержка при включении реле
   jsonConfig["RELAY1"]["DELAY_OFF"]   = 5;                          // Задержка при отключении реле

// Настройка реле2   
   jsonConfig["RELAY2"]["MODE"]        = RELAY_NORMAL;               // Режим работы реле
   jsonConfig["RELAY2"]["INVERSE"]     = false;                      // Флаг иверсии реле
   jsonConfig["RELAY2"]["T_PULSE"]     = 1;                          // Длительность импульса при импульсном режиме
   jsonConfig["RELAY2"]["T_PAUSE"]     = 1;                          // Длительность паузы при импульсном режиме
   jsonConfig["RELAY2"]["DELAY_ON"]    = 1;                          // Залержка при включении реле
   jsonConfig["RELAY2"]["DELAY_OFF"]   = 1;                          // Задержка при отключении реле



}

void configSave(){
   File f = SPIFFS.open(CONFIG_JSON, FILE_WRITE);
   serializeJson(jsonConfig, f);
   f.close();

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
      if( jsonConfig.isNull()     ) save_flag = true; 
   }
   if( save_flag ){
       saveDefault();
       saveSave();
   }
}

void saveDefault(){
   jsonSave.clear();
   jsonSave["DISTANCE"]    = 0;
   jsonSave["STATE_ON"]    = SENSOR_GROUND_STATE;
   jsonSave["BOOT_COUNT"]  = 0;


}

void saveSave(){
   File f = SPIFFS.open(SAVE_JSON, FILE_WRITE);
   serializeJson(jsonSave, f);
   f.close();

   savePrint("Save");  
}

void saveSet(float _dist, bool _on){
   jsonSave["DISTANCE"]    = _dist;
   jsonSave["STATE_ON"]    = _on;
   saveSave();
   
}

uint16_t saveCount(){
   uint16_t _count = 0;
   if( jsonSave["BOOT_COUNT"] == "" ){
      jsonSave["BOOT_COUNT"] = _count;   
   }
   else {
      _count = jsonSave["BOOT_COUNT"].as<uint16_t>();
//      _count++;
      jsonSave["BOOT_COUNT"] = _count+1;
   }
   saveSave();
   return _count;
}
