#ifndef Config_h
#define Config_h
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WebSocketClient.h>

#define SONAR_DYP_ME007Y    0
#define SONAR_SERIAL        0
#define SONAR_JSN_SR04T     1
#define SONAR_TRIG_ECHO     1
#define SONAR_JSN_SR04TV2   2
#define SONAR_JSN_SR04M_2   3
#define LIDAR_TFMINI_I2C    10
#define LIDAR_TFLUNA_I2C    11

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

#define SONAR_SENSOR_TYPE SONAR_JSN_SR04T
//#define SONAR_SENSOR_TYPE LIDAR_TFLUNA_I2C
//#define SONAR_SENSOR_TYPE SONAR_JSN_SR04M_2
//#define SONAR_SENSOR_TYPE    SONAR_DYP_ME007Y


// Изменение расстояния до датчика в мм, когда проиходит эмуляция нажатия кнопки
//#define LimitDistance            500
// Изменение температуры и влажности, года происходит отсылка
#define LimitTemp                1
#define LimitHum                 5

// Интервал отправки на сервер в мс
// Минимальное время опросв датчиков
#define LoopInterval             1000
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
#define WS_PixelCount     30   //Число светодиодов
#define WS_PIN            13
#define WS_TM_DEFAULT     250 //Залержка между эффектами "По умолчанию"
#define WS_MODE_DEFAULT   1   //Режим "По умолчанию"
#define WS_MAX_BRIGHTNESS 255 //Максимальная яркость (0-255)


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
