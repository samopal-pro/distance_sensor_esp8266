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
       configDefault();
       configSave();
   }
}

String deviceNmae( char *_name){
    String s = _name;
    if( strlen(serNo) >0 )s.replace("2025",serNo);
    return s;
}

void configDefault(){
   jsonConfig.clear();
   int n = 0;
// Системные параметры
   jsonConfig["CONFIG_V"]              = CONFIG_V;                //Имя точки доступа устройства  
   jsonConfig["SYSTEM"]["NAME"]        = deviceNmae(DEVICE_NAME);                //Имя точки доступа устройства  
   jsonConfig["SYSTEM"]["AP_START"]    = AP_ALWAYS;  
   jsonConfig["SYSTEM"]["PASS0"]       = DEVICE_PASS0;               //Пароль суперадминистратора
   jsonConfig["SYSTEM"]["PASS1"]       = DEVICE_PASS1;               //Пароль администратора
   jsonConfig["SYSTEM"]["PASSS"]       = DEVICE_PASSS;               //Пароль светофорбока (мегоадминистратора)
// Параметры моединения WiFi
   jsonConfig["WIFI"]["NAME"]              = "";                         //Имя сети WiFi
   jsonConfig["WIFI"]["PASS"]              = "";                         //Пароль сети WiFi
   jsonConfig["WIFI"]["DHCP"]              = true;                       //Брать парметры сети по DHCP
   jsonConfig["WIFI"]["IP"]["ADDR"]        = "192.168.1.10";             //IP адрес при стаическом режиме
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
   jsonConfig["TB"]["TOKEN"]               = "";

   jsonConfig["LORA"]["ENABLE"]            = false;                      //Посылать информацию по Лоре

   
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
   jsonConfig["RGB1"]["BRIGHTNESS"]    = 5;                          // Яркость ленты 1-10
   jsonConfig["RGB1"]["FREE"]          = COLOR_FREE_DEFAULT;
   jsonConfig["RGB1"]["FREE_BLINK"]    = COLOR_BLINK_DEFAULT;
   jsonConfig["RGB1"]["IS_FREE_BLINK"] = true;
   jsonConfig["RGB1"]["BUSY"]          = COLOR_BUSY_DEFAULT;
   jsonConfig["RGB1"]["IS_NAN_MODE"]   = true;
   jsonConfig["RGB1"]["NAN_MODE"]      = NAN_VALUE_IGNORE;

// Настройка подсветки датчика RGB2      
   jsonConfig["RGB2"]["BRIGHTNESS"]    = 5;                          // Яркость ленты 1-10
   jsonConfig["RGB2"]["FREE"]          = COLOR_FREE_DEFAULT;
   jsonConfig["RGB2"]["FREE_BLINK"]    = COLOR_BLINK_DEFAULT;
   jsonConfig["RGB2"]["IS_FREE_BLINK"] = true;
   jsonConfig["RGB2"]["BUSY"]          = COLOR_BUSY_DEFAULT;
   jsonConfig["RGB2"]["IS_NAN_MODE"]   = true;
   jsonConfig["RGB2"]["NAN_MODE"]      = NAN_VALUE_IGNORE;
   jsonConfig["RGB2"]["IS_MP3"]        = true;
   jsonConfig["RGB2"]["MP3"]           = COLOR_MP3_DEFAULT;

// Настройка плеера MP3      
   jsonConfig["MP3"]["VOLUME"]            = 30;                          // Громкость 0-30
   jsonConfig["MP3"]["EQ"]                = 0;                           // Режим эквалайзера 0-5
   jsonConfig["MP3"]["SHORT_MSG"]         = false;
// Автомобиль заехал
   jsonConfig["MP3"]["BUSY"]["ENABLE"]    = true;
//   jsonConfig["MP3"]["BUSY"]["DIR"]       = 1;
   jsonConfig["MP3"]["BUSY"]["NUM"]       = 1;
   jsonConfig["MP3"]["BUSY"]["DELAY"]     = 5;
   jsonConfig["MP3"]["BUSY"]["LOOP"]      = false;
   jsonConfig["MP3"]["BUSY"]["COLOR"]     = COLOR_MP3_1;
   jsonConfig["MP3"]["BUSY"]["COLOR_TM"]  = 4;
// Датчик NAN (пена) 
   jsonConfig["MP3"]["NAN"]["ENABLE"]     = true;
//   jsonConfig["MP3"]["NAN"]["DIR"]        = 1;
   jsonConfig["MP3"]["NAN"]["NUM"]        = 2;
   jsonConfig["MP3"]["NAN"]["DELAY"]      = 10;
   jsonConfig["MP3"]["NAN"]["LOOP"]       = false;
   jsonConfig["MP3"]["NAN"]["COLOR"]      = COLOR_MP3_2;
   jsonConfig["MP3"]["NAN"]["COLOR_TM"]   = 4;
// Слишком долго автомобиль в боксе
   jsonConfig["MP3"]["BUSY1"]["ENABLE"]   = true;
//   jsonConfig["MP3"]["BUSY1"]["DIR"]      = 1;
   jsonConfig["MP3"]["BUSY1"]["NUM"]      = 3;
   jsonConfig["MP3"]["BUSY1"]["DELAY"]    = 900;
   jsonConfig["MP3"]["BUSY1"]["LOOP"]     = true;
   jsonConfig["MP3"]["BUSY1"]["COLOR"]    = COLOR_MP3_2;
   jsonConfig["MP3"]["BUSY1"]["COLOR_TM"] = 4;
// Слишком долго автомобиль в боксе или датчик "залип"
   jsonConfig["MP3"]["BUSY2"]["ENABLE"]      = true;
//   jsonConfig["MP3"]["BUSY2"]["DIR"]      = 1;
   jsonConfig["MP3"]["BUSY2"]["NUM"]         = 4;
   jsonConfig["MP3"]["BUSY2"]["DELAY"]       = 1800;
   jsonConfig["MP3"]["BUSY2"]["LOOP"]        = true;
   jsonConfig["MP3"]["BUSY2"]["COLOR"]       = COLOR_MP3_2;
   jsonConfig["MP3"]["BUSY2"]["COLOR_TM"]    = 4;
// После выезда NAN
   jsonConfig["MP3"]["FREE_NAN"]["ENABLE"]   = true;
//   jsonConfig["MP3"]["FREE_NAN"]["DIR"]   = 1;
   jsonConfig["MP3"]["FREE_NAN"]["NUM"]      = 5;
   jsonConfig["MP3"]["FREE_NAN"]["DELAY"]    = 4;
   jsonConfig["MP3"]["FREE_NAN"]["LOOP"]     = true;
   jsonConfig["MP3"]["FREE_NAN"]["COLOR"]    = COLOR_MP3_2;
   jsonConfig["MP3"]["FREE_NAN"]["COLOR_TM"] = 4;
// Автомобиль выехал
   jsonConfig["MP3"]["FREE"]["ENABLE"]       = true;
//   jsonConfig["MP3"]["FREE"]["DIR"]       = 1;
   jsonConfig["MP3"]["FREE"]["NUM"]          = 6;
   jsonConfig["MP3"]["FREE"]["DELAY"]        = 5;
   jsonConfig["MP3"]["FREE"]["LOOP"]         = false;
   jsonConfig["MP3"]["FREE"]["COLOR"]        = COLOR_MP3_1;
   jsonConfig["MP3"]["FREE"]["COLOR_TM"]     = 4;

//   jsonConfig["MP3"]["SYSTEM"]["DIR"] = 1;                           // Номер папки с системными звуками



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

// Стартовые настройки
//   jsonConfig["MP3"]["ADD"]["DIR"]     = 2;                          // Каталог с дорожками дополнительных сообщений
   jsonConfig["BOOT0"]["DELAY0"]       = 10;                         // Задержка начала первой калибровки при нажатии "Активировать"
   jsonConfig["BOOT0"]["DELAY10"]      = 10;                         // Задержка начала первой калибровки при удержании 10 сек кнопки 
   
   jsonConfig["MP3"]["100"]["ENABLE"]  = true; 
   jsonConfig["MP3"]["99"]["ENABLE"]   = true; 
   jsonConfig["MP3"]["99"]["DELAY"]    = 10; 
   jsonConfig["MP3"]["99"]["COLOR_TM"] = 10; 
   
   jsonConfig["MP3"]["98"]["ENABLE"]   = true; 
   jsonConfig["MP3"]["97"]["ENABLE"]   = true; 
   jsonConfig["MP3"]["96"]["ENABLE"]   = true; 
   jsonConfig["MP3"]["93"]["ENABLE"]   = true; 
//   jsonConfig["MP3"]["93"]["COLOR_TM"] = 10; 
   jsonConfig["MP3"]["92"]["ENABLE"]   = true; 
//   jsonConfig["MP3"]["92"]["COLOR_TM"] = 10; 

   jsonConfig["MP3"]["92"]["ENABLE"]   = true; 
   jsonConfig["MP3"]["89"]["ENABLE"]   = true; 
   jsonConfig["MP3"]["88"]["ENABLE"]   = true; 
   jsonConfig["MP3"]["70"]["ENABLE"]   = true; 


}

void configSave(){
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
   jsonSave["MSG_AUTH"]    = true;


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
