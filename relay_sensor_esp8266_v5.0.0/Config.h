#ifndef Config_h
#define Config_h
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
//#include <WebSocketClient.h>

#define FW_VERSION   "VERSION 5.0.6"
#define DEVICE_NAME  "693_SVETOFORBOX.RU_192.168.4.1"
#define DEVICE_ADMIN "superadmin"
#define DEVICE_OPER  "admin"
//#define WIFI_SAV
/*                
Затычка на глючащие датчики SONAR_JSN_SR04TV2
При дистанции свыше 3000 они начинают показывать 
постоянно убывающее значение 
При фиксации этой екндкнции устанавливается дистанция 
EA_Config.ZeroDistance = 11111
Очень не рекомендуется использовать, так как датчики
иногда что-то показывают и возникают ложные срабатывания
*/ 
//#define SONAR_FAKE

enum T_SENSOR_TYPE {
  SONAR_SERIAL  = 0, // Все платы работающие по протоколу UART
  SONAR_SR04T   = 1, // Все платы работающие по протоколу Trig/Echo
  SONAR_SR04TV2 = 2, // Глючные платы  SR04TV2 у которых плывет значение расстояния
  SONAR_SR04TM2 = 3, // Последние платы SR04M2 у которых увелмчено время импуься с 10 до 500мс и установлено ограничение на дистанцию 5000
  SONAR_TFMINI  = 10,// LiDAR TF Mini Plus по I2C
  SONAR_TFLUNA  = 11 // LiDAR TF Luna по I2C
};

enum T_NAN_VALUE_FLAG {
  NAN_VALUE_IGNORE  = 1,  
  NAN_VALUE_FREE    = 2,
  NAN_VALUE_BUSY    = 3
};
  
enum T_MEASURE_TYPE {
  MEASURE_TYPE_NORMAL  = 1, //Срабатываение датчика в обычном режиме (уменьшение дистанции)
  MEASURE_TYPE_OUTSIDE = 2, //Срабатывание датчика если вне интервала
  MEASURE_TYPE_INSIDE  = 3  //Срабатывание датчика если внутри интервала
};

//#define DEFAULT_SENSOR_TYPE SONAR_SERIAL  // Старые сенсоры ME007Y и другие платы работающие по протоколу SERIAL
#define DEFAULT_SENSOR_TYPE SONAR_SR04T   // Большинство датчиков SR04T (работаю посылая сигнал на Trih и ловя отраженный сигнал на Echo)
//#define DEFAULT_SENSOR_TYPE SONAR_SR04TV2 // Глючные платы  SR04TV2 у которых плывет значение расстояния
//#define DEFAULT_SENSOR_TYPE SONAR_SR04TM2 // Последние платы SR04M2 у которых увелмчено время импуься с 10 до 500мс и установлено ограничение на дистанцию 5000
//#define DEFAULT_SENSOR_TYPE SONAR_TFMINI  // LiDAR TF Mini Plus по I2C
//#define DEFAULT_SENSOR_TYPE ONAR_TFLUNA   // LiDAR TF Luna по I2C

#define DEFAULT_NAN_VALUE_FLAG NAN_VALUE_IGNORE
#define DEFAULT_MEASURE_TYPE MEASURE_TYPE_NORMAL

extern T_SENSOR_TYPE sensorType;
//extern T_NAN_VALUE_FLAG nanValueFlag;

// Изменение расстояния до датчика в мм, когда проиходит эмуляция нажатия кнопки
//#define LimitDistance            500
// Изменение температуры и влажности, года происходит отсылка
#define LimitTemp                1
#define LimitHum                 5

// Интервал отправки на сервер в мс
// Минимальное время опросв датчиков
//#define LoopInterval             1000
//#define LoopIntervalAP           3000
#define GetStatusInterval        10000
// Интервал между запросоми состояния
#define SendInterval             60000
// Интервал между запросами записи в архив
#define SendArhInterval          10000
#define HTTP_ERROR_COUNT         20
// Перегружать через заданное количество ошибок связи, если сработала кнопка. 0 - отключено
#define RESET_ERR_COUNT          10

// Интервал принудительной перезагрузки (0 - перезагрузка отключена)
#define ResetInterval            7200000


// Пины на ультразвуковой датчик
#define PinDistanceTrig          5
#define PinDistanceEcho          4
#define PinRelay                 -1
#define PinCalibrateSonar        0
#define SonarGroudState          false
// Пин на датчик температуры влажности DHT22 (2)
#define PinDHT22                 2
#define PinController            16
// Пины I2C интерфейса для часов реального времени
#define PinRTC_SDA               14
#define PinRTC_SCL               12


// Пин кнопки
//for old sensor
#define PinButton                -1
//for new sensor                 5
//#define PinButton              -1

// Время эмуляции нажатия кнопки в мс. Если значение меньше или равно 0, то кнопка работает в реальном режиме
//#define TimeButtonEmulate        100000

//С какой попытки срабатывает кнопка на включение(0 сразу)
//#define ButtonBounceOn                0
//С какой попытки срабатывает кнопка на выключение(0 сразу)
//#define ButtonBounceOff               0
//Количество милисекунд до срабатывания на включение (0 сразу)
#define ButtonTimeOutOn             500
//Количество милисекунд до срабатывания на включение (0 сразу)
#define ButtonTimeOutOff            500
// Минимальный интервал между двумя включениями, когда не запичываетя архив,сек (0-отключено)
#define ButtonArhIntervalOn        120
// Минимальный интервал между двумя выключениями, когда не запичываетя архив,сек (0-отключено)
#define ButtonArhIntervalOff       0

/*
   Конфигурация светодиодной ленты
*/
#define LED_COUNT         13   //Число светодиодов
#define PinLed            13


// Pin WDT
#define WDTInterval           10800000
#define PinWDT                -1

// true - работаем через WEBSOCKET, false - через HTTP
#define WebSocketFlag               false
// Проверять ответ от сервера при посылке параметров на "STAT 200"
// true - проверяется, false - нет
#define CheckRequest                 true
// true - работать по HTTP, если WebSocket глюкнул, false - нет
#define ReserveHttp                 false


#define WS_PATH                  "/"
#define HTTP_PATH                 "/api/1/sensor/99/"



#endif
