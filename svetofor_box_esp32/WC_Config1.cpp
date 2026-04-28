#include "WC_Config.h"

char CONFIG_JSON[20];

void configSet(T_CONFIG _config){
  
   if( _config != CFG_NONE ){
       jsonSave["CONFIG"] = _config;
       saveSave();
   }
   switch(jsonSave["CONFIG"].as<int>()){
      case CFG_CARWASH : strcpy(CONFIG_JSON, CONFIG_JSON1); break;
      case CFG_PARKING : strcpy(CONFIG_JSON, CONFIG_JSON2); break;
      default: strcpy(CONFIG_JSON, CONFIG_JSON_TEST); 
   }   
  
}


void configDefault(){
   configDefault1();
   switch(jsonSave["CONFIG"].as<int>()){
      case CFG_CARWASH : configDefault1(); break;
      case CFG_PARKING : configDefault2(); break;
      default: configDefaultTest(); 
   }   
    
}

void configDefault1(){
   configDefaultGeneral();
   jsonConfig["SENSOR"]["TYPE"]        = SENSOR_SR04_75; //Тип датчика 
   jsonConfig["WIFI"]["NAME1"]          = "svetoforbox.ru";                         //Имя сети WiFi
   jsonConfig["WIFI"]["PASS1"]          = "89060725500";                         //Пароль сети WiFi
//   jsonConfig["WIFI"]["POWER"]         = WIFI_POWER_21dBm;           //Уровень сигнала WiFi

   
// Параметры моединения WiFi
   jsonConfig["RGB2"]["FREE"]          = 0xFFFFFF;
   jsonConfig["NET"]["DOGOVOR_ID"]     = "0000";            
   jsonConfig["CRM_MOSCOW"]["ENABLE"]  = true;                      //посылать информацию на CRM-MOSCOW
   jsonConfig["TB"]["ENABLE"]          = true;                      //Посылать информацию в ThingsBoard



   jsonConfig["RELAY1"]["MODE"]        = RELAY_NORMAL;               // Режим работы реле
   jsonConfig["RELAY1"]["T_PULSE"]     = 1;                          // Длительность импульса при импульсном режиме
   jsonConfig["RELAY1"]["T_PAUSE"]     = 1;                          // Длительность паузы при импульсном режиме
   jsonConfig["RELAY1"]["DELAY_ON"]    = 1;                          // Залержка при включении реле
   jsonConfig["RELAY1"]["DELAY_OFF"]   = 1;                          // Задержка при отключении реле
   jsonConfig["RELAY2"]["MODE"]        = RELAY_NORMAL;               // Режим работы реле
   jsonConfig["RELAY2"]["T_PULSE"]     = 1;                          // Длительность импульса при импульсном режиме
   jsonConfig["RELAY2"]["T_PAUSE"]     = 1;                          // Длительность паузы при импульсном режиме
   jsonConfig["RELAY2"]["DELAY_ON"]    = 1;                          // Залержка при включении реле
   jsonConfig["RELAY2"]["DELAY_OFF"]   = 1;       

   jsonConfig["RGB1"]["BRIGHTNESS"]    = 10;                          // Яркость ленты 1-10
   jsonConfig["RGB2"]["BRIGHTNESS"]    = 10;                          // Яркость ленты 1-10
   jsonConfig["MP3"]["VOLUME"]         = 30;                          // Громкость 0-30

  
   jsonConfig["MP3"]["BUSY"]["DELAY"]     = 8;
   jsonConfig["MP3"]["BUSY"]["LOOP"]      = false;
   jsonConfig["MP3"]["NAN"]["DELAY"]      = 30;
   jsonConfig["MP3"]["NAN"]["LOOP"]       = false;
   jsonConfig["MP3"]["BUSY1"]["DELAY"]    = 900;
   jsonConfig["MP3"]["BUSY1"]["LOOP"]     = true;
   jsonConfig["MP3"]["BUSY1"]["DELAY"]    = 1800;
   jsonConfig["MP3"]["BUSY1"]["LOOP"]     = true;
   jsonConfig["MP3"]["FREE_NAN"]["DELAY"] = 5;
   jsonConfig["MP3"]["FREE_NAN"]["LOOP"]  = false;
   jsonConfig["MP3"]["FREE"]["DELAY"]     = 3;
   jsonConfig["MP3"]["FREE"]["LOOP"]      = false;
   jsonConfig["MP3"]["BTN_ADD1"]["TIMER"] = 180;
   jsonConfig["MP3"]["BTN_ADD2"]["TIMER"] = 300;
   jsonConfig["MP3"]["BTN_ADD"]["ENABLE"] = true;


}

void configDefault2(){
   configDefaultGeneral();
   jsonConfig["RGB1"]["FREE"]          = COLOR_FREE2;
   jsonConfig["RGB2"]["FREE"]          = 0xFFFFFF;
   jsonConfig["RGB1"]["NAN_MODE"]      = NAN_VALUE_FREE;
   jsonConfig["SENSOR"]["INSTALL"]     = INSTALL_TYPE_NORMAL; //Тип установки/срабатывания датчика
   jsonConfig["SENSOR"]["DIST_GROUND"] = 2700;       //Высота установки датчика (расстояние до пола)
   jsonConfig["SENSOR"]["DIST_LIMIT"]  = 500;        //Если разница между текущим показанием и высатой установки больше этого значения, то датчик показывает занято
   jsonConfig["SENSOR"]["TYPE"]        = SENSOR_SR04TM2; //Тип датчика
   jsonConfig["WIFI"]["NAME1"]          = "parkingbox.ru";                         //Имя сети WiFi
   jsonConfig["WIFI"]["PASS1"]          = "89060725500";                         //Пароль сети WiFi
   jsonConfig["NET"]["DOGOVOR_ID"]     = "0000";            
   jsonConfig["CRM_MOSCOW"]["ENABLE"]  = false;                      //посылать информацию на CRM-MOSCOW
   jsonConfig["TB"]["ENABLE"]          = false;                      //Посылать информацию в ThingsBoard

   jsonConfig["RELAY1"]["MODE"]        = RELAY_NORMAL;               // Режим работы реле
   jsonConfig["RELAY1"]["T_PULSE"]     = 1;                          // Длительность импульса при импульсном режиме
   jsonConfig["RELAY1"]["T_PAUSE"]     = 1;                          // Длительность паузы при импульсном режиме
   jsonConfig["RELAY1"]["DELAY_ON"]    = 1;                          // Залержка при включении реле
   jsonConfig["RELAY1"]["DELAY_OFF"]   = 1;                          // Задержка при отключении реле
   jsonConfig["RELAY2"]["MODE"]        = RELAY_NORMAL;               // Режим работы реле
   jsonConfig["RELAY2"]["T_PULSE"]     = 1;                          // Длительность импульса при импульсном режиме
   jsonConfig["RELAY2"]["T_PAUSE"]     = 1;                          // Длительность паузы при импульсном режиме
   jsonConfig["RELAY2"]["DELAY_ON"]    = 1;                          // Залержка при включении реле
   jsonConfig["RELAY2"]["DELAY_OFF"]   = 1;       

   jsonConfig["MP3"]["BUSY"]["ENABLE"]     = true;
   jsonConfig["MP3"]["NAN"]["ENABLE"]      = false;
   jsonConfig["MP3"]["BUSY1"]["ENABLE"]    = false;
   jsonConfig["MP3"]["BUSY2"]["ENABLE"]    = false;
   jsonConfig["MP3"]["FREE_NAN"]["ENABLE"] = false;
   jsonConfig["MP3"]["FREE"]["ENABLE"]     = true;


   jsonConfig["RGB1"]["BRIGHTNESS"]    = 10;                          // Яркость ленты 1-10
   jsonConfig["RGB2"]["BRIGHTNESS"]    = 10;                          // Яркость ленты 1-10
   jsonConfig["MP3"]["VOLUME"]         = 23;                          // Громкость 0-30
   jsonConfig["MP3"]["BUSY"]["DELAY"]     = 10;
   jsonConfig["MP3"]["BUSY"]["LOOP"]      = false;
   jsonConfig["MP3"]["NAN"]["DELAY"]      = 5;
   jsonConfig["MP3"]["NAN"]["LOOP"]       = false;
   jsonConfig["MP3"]["BUSY1"]["DELAY"]    = 50;
   jsonConfig["MP3"]["BUSY1"]["LOOP"]     = false;
   jsonConfig["MP3"]["BUSY1"]["DELAY"]    = 25;
   jsonConfig["MP3"]["BUSY1"]["LOOP"]     = false;
   jsonConfig["MP3"]["FREE_NAN"]["DELAY"] = 2;
   jsonConfig["MP3"]["FREE_NAN"]["LOOP"]  = false;
   jsonConfig["MP3"]["FREE"]["DELAY"]     = 3;
   jsonConfig["MP3"]["FREE"]["LOOP"]      = false;
   jsonConfig["MP3"]["BTN_ADD1"]["TIMER"] = 1;
   jsonConfig["MP3"]["BTN_ADD2"]["TIMER"] = 15;

   jsonConfig["RGB1"]["STAT_AP"]        = LED_STAT_AP2;               // Номер светодиода статуса AP
   jsonConfig["RGB1"]["STAT_STA"]       = LED_STAT_STA2;              // Номер светодиода статуса STA



}

void configDefaultTest(){
   configDefaultGeneral();
   jsonConfig["SENSOR"]["TYPE"]        = SENSOR_SR04_75; //Тип датчика 

#if defined(SAV_CONFIG)
   jsonConfig["WIFI"]["NAME1"]          = "ASUS_58_2G";                         //Имя сети WiFi
   jsonConfig["WIFI"]["PASS1"]          = "sav59vas";                         //Пароль сети WiFi
   jsonConfig["WIFI"]["DHCP"]           = false;                       //Брать парметры сети по DHCP
   jsonConfig["WIFI"]["IP"]["ADDR"]     = "192.168.1.32";             //IP адрес при стаическом режиме

#else
   jsonConfig["WIFI"]["NAME1"]          = "910";                         //Имя сети WiFi
   jsonConfig["WIFI"]["PASS1"]          = "89060725500";                         //Пароль сети WiFi
#endif

   jsonConfig["RGB1"]["FREE"]          = COLOR_FREE1;
   jsonConfig["RGB1"]["NAN_MODE"]      = NAN_VALUE_IGNORE;
   jsonConfig["SENSOR"]["INSTALL"]     = INSTALL_TYPE_NORMAL; //Тип установки/срабатывания датчика
   jsonConfig["SENSOR"]["DIST_GROUND"] = 270;       //Высота установки датчика (расстояние до пола)
   jsonConfig["SENSOR"]["DIST_LIMIT"]  = 270;        //Если разница между текущим показанием и высатой установки больше этого значения, то датчик показывает занято
   jsonConfig["SENSOR"]["TYPE"]        = SENSOR_SR04TM2; //Тип датчика
   jsonConfig["NET"]["DOGOVOR_ID"]     = "8888";            
   jsonConfig["CRM_MOSCOW"]["ENABLE"]  = true;                      //посылать информацию на CRM-MOSCOW
   jsonConfig["TB"]["ENABLE"]          = true;                      //Посылать информацию в ThingsBoard

   jsonConfig["RELAY1"]["MODE"]        = RELAY_NORMAL;               // Режим работы реле
   jsonConfig["RELAY1"]["T_PULSE"]     = 1;                          // Длительность импульса при импульсном режиме
   jsonConfig["RELAY1"]["T_PAUSE"]     = 1;                          // Длительность паузы при импульсном режиме
   jsonConfig["RELAY1"]["DELAY_ON"]    = 1;                          // Залержка при включении реле
   jsonConfig["RELAY1"]["DELAY_OFF"]   = 1;                          // Задержка при отключении реле
   jsonConfig["RELAY2"]["MODE"]        = RELAY_NORMAL;               // Режим работы реле
   jsonConfig["RELAY2"]["T_PULSE"]     = 1;                          // Длительность импульса при импульсном режиме
   jsonConfig["RELAY2"]["T_PAUSE"]     = 1;                          // Длительность паузы при импульсном режиме
   jsonConfig["RELAY2"]["DELAY_ON"]    = 1;                          // Залержка при включении реле
   jsonConfig["RELAY2"]["DELAY_OFF"]   = 1;       

   jsonConfig["MP3"]["SHORT_MSG"]          = true;
   jsonConfig["RGB1"]["BRIGHTNESS"]        = 10;                          // Яркость ленты 1-10
   jsonConfig["RGB2"]["BRIGHTNESS"]        = 10;                          // Яркость ленты 1-10
   jsonConfig["MP3"]["VOLUME"]             = 13;                          // Громкость 0-30

   jsonConfig["MP3"]["BUSY"]["ENABLE"]     = true;
   jsonConfig["MP3"]["NAN"]["ENABLE"]      = true;
   jsonConfig["MP3"]["BUSY1"]["ENABLE"]    = true;
   jsonConfig["MP3"]["BUSY2"]["ENABLE"]    = true;
   jsonConfig["MP3"]["FREE_NAN"]["ENABLE"] = true;
   jsonConfig["MP3"]["FREE"]["ENABLE"]     = true;

   jsonConfig["MP3"]["BUSY"]["DELAY"]     = 1;
   jsonConfig["MP3"]["BUSY"]["LOOP"]      = false;
   jsonConfig["MP3"]["NAN"]["DELAY"]      = 2;
   jsonConfig["MP3"]["NAN"]["LOOP"]       = false;
   jsonConfig["MP3"]["BUSY1"]["DELAY"]    = 50;
   jsonConfig["MP3"]["BUSY1"]["LOOP"]     = false;
   jsonConfig["MP3"]["BUSY1"]["DELAY"]    = 25;
   jsonConfig["MP3"]["BUSY1"]["LOOP"]     = false;
   jsonConfig["MP3"]["FREE_NAN"]["DELAY"] = 2;
   jsonConfig["MP3"]["FREE_NAN"]["LOOP"]  = false;
   jsonConfig["MP3"]["FREE"]["DELAY"]     = 1;
   jsonConfig["MP3"]["FREE"]["LOOP"]      = false;
   jsonConfig["MP3"]["BTN_ADD1"]["TIMER"] = 1;
   jsonConfig["MP3"]["BTN_ADD2"]["TIMER"] = 15;

}

void configDefaultGeneral(){
   int n = 0;
   jsonConfig.clear();
   jsonConfig["CONFIG_V"]              = CONFIG_V;                //Имя точки доступа устройства  
//   configDefaultGeneral();
// Системные параметры
   jsonConfig["SENSOR"]["TYPE"]        = DEFAULT_SENSOR_TYPE; //Тип установки/срабатывания датчика
   jsonConfig["SENSOR"]["INSTALL"]     = DEFAULT_SENSOR_INSTALL_TYPE; //Тип установки/срабатывания датчика
   jsonConfig["SYSTEM"]["NAME"]        = deviceName();                //Имя точки доступа устройства  
   jsonConfig["SYSTEM"]["AP_START"]    = AP_ALWAYS;  
   jsonConfig["SYSTEM"]["PASS0"]       = DEVICE_PASS0;               //Пароль суперадминистратора
   jsonConfig["SYSTEM"]["PASS1"]       = DEVICE_PASS1;               //Пароль администратора
   jsonConfig["SYSTEM"]["PASSS"]       = DEVICE_PASSS;               //Пароль светофорбока (мегоадминистратора)
// Параметры подключения к CRM.MOSCOW
//   jsonConfig["NET"]["ENABLE"]            = false;                      //Коннектится по WiFi

   jsonConfig["WIFI"]["NAME"]              = "";                         //Имя сети WiFi
   jsonConfig["WIFI"]["PASS"]              = "";                         //Пароль сети WiFi
   jsonConfig["WIFI"]["NAME1"]             = "";                         //Имя сети WiFi
   jsonConfig["WIFI"]["PASS1"]             = "";                         //Пароль сети WiFi
   jsonConfig["WIFI"]["MODE"]              = STA_AUTO;                         //Имя сети WiFi
  
 
   jsonConfig["WIFI"]["POWER"]             = WIFI_POWER_21dBm;           //Уровень сигнала WiFi
   jsonConfig["WIFI"]["DHCP"]              = true;                       //Брать парметры сети по DHCP
   jsonConfig["WIFI"]["IP"]["ADDR"]        = "192.168.1.10";             //IP адрес при стаическом режиме
   jsonConfig["WIFI"]["IP"]["MASK"]        = "255.255.255.0";            //IP маска при стаическом режиме
   jsonConfig["WIFI"]["IP"]["GW"]          = "192.168.1.1";              //IP адрес шлюза при стаическом режиме
   jsonConfig["WIFI"]["IP"]["DNS"]         = "8.8.8.8";                  //IP адрес DNS при стаическом режиме

   jsonConfig["NET"]["DOGOVOR_ID"]         = "0000";            
   jsonConfig["NET"]["BOX_ID"]             = "1";                     
   jsonConfig["NET"]["T_SEND"]             = 1000;                   //Таймаут отправки на сервер            
   jsonConfig["NET"]["T_RETRY"]            = 10;                     //Таймаут повторной отправки если статус HTTP не 200            


   jsonConfig["CRM_MOSCOW"]["ENABLE"]      = false;                      //посылать информацию на CRM-MOSCOW
   jsonConfig["CRM_MOSCOW"]["SERVER"]      = "crm.moscow";             
   jsonConfig["CRM_MOSCOW"]["PORT"]        = 8001;

   jsonConfig["HTTP"]["ENABLE"]            = false;                      //посылать информацию на GATEWAY по HTTP
//   jsonConfig["HTTP"]["SERVER"]            = "";             
   jsonConfig["HTTP"]["SERVERS"][0]        = "";
//   jsonConfig["HTTP"]["PORT"]              = 80;

   jsonConfig["TB"]["ENABLE"]              = false;                      //Посылать информацию в ThingsBoard
   jsonConfig["TB"]["GATEWAY"]             = false;                      //Отправка через шлюз
   jsonConfig["TB"]["SERVER"]              = "109.172.115.70";             
   jsonConfig["TB"]["PORT"]                = 8088;
   jsonConfig["TB"]["TOKEN"]               = "";

   jsonConfig["LORA"]["ENABLE"]            = false;                      //Посылать информацию по Лоре
   jsonConfig["LORA"]["GATEWAY"]           = (uint64_t)0;                //Посылать информацию по Лоре

   
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

  jsonConfig["RGB1"]["STAT_AP"]        = LED_STAT_AP1;               // Номер светодиода статуса AP
  jsonConfig["RGB1"]["STAT_STA"]       = LED_STAT_STA1;              // Номер светодиода статуса STA


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

#if defined(IS_BTN_ADD)
   jsonConfig["MP3"]["BTN_ADD"]["ENABLE"]    = false;
   jsonConfig["MP3"]["BTN_ADD"]["INVERSE"]   = false;
   jsonConfig["MP3"]["BTN_ADD1"]["NUM"]      = 7;
   jsonConfig["MP3"]["BTN_ADD1"]["TIMER"]    = 10;
   jsonConfig["MP3"]["BTN_ADD2"]["NUM"]      = 8;
   jsonConfig["MP3"]["BTN_ADD2"]["TIMER"]    = 300;
#endif

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
   jsonConfig["MP3"]["60"]["ENABLE"]   = true;                  //Блок подключения к WiFi

}

