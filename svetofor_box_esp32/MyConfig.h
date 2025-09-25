#ifndef MY_CONFIG
#define MY_CONFIG

// Выдача сообщение сенсоров 1- иницилизация и ошибки, 2 - измерение параметров
#define DEBUG_SENSORS        2
#define DEBUG_SERIAL

#define SOFTWARE_V           "10.0.0"
#define HARDWARE_V           "10.0.0"


#define SENSOR_TYPE_NONE     0
#define SENSOR_SR04T         1 //Типовой сенсор TGIG/ECHO
#define SENSOR_SR04TM2       2 //Типовой сенсор TGIG/ECHO
//#define SENSOR_A21_I2C       20 //A21 I2C сенсор подключен к основному разъему (SCL = TRIG, SDA = ECHO )
#define SENSOR_TFLUNA_I2C    10 //TFLina I2C сенсор подключен к основному разъему (SCL = TRIG, SDA = ECHO )
#define SENSOR_TFMINI_I2C    11 //TFMiniPlus I2C сенсор подключен к основному разъему (SCL = TRIG, SDA = ECHO )

#define SENSOR_A21_I2C       20 //A21 I2C сенсор подключен к I2C разъему

/*
enum T_SENSOR_TYPE {
//  SONAR_SERIAL  = 0, // Все платы работающие по протоколу UART
  SENSOR_SR04T   = 1, // Все платы работающие по протоколу Trig/Echo
//  SONAR_SR04TV2  = 3, // Глючные платы  SR04TV2 у которых плывет значение расстояния
  SENSOR_SR04TM2 = 2, // Последние платы SR04M2 у которых увелмчено время импуься с 10 до 500мс и установлено ограничение на дистанцию 5000
  SENSOR_TFLUNA_I2C  = 10, // LiDAR TF Mini Plus по I2C
  SENSOR_TFMINI_I2C  = 11, // LiDAR TF Luna по I2C
  SENSOR_A21_I2C     = 23  //A21 I2C сенсор подключен к I2C разъему
};
*/

#define DEFAULT_SENSOR_TYPE SENSOR_SR04T 

#define PIN_LORA_MOSI        23
#define PIN_LORA_MISO        19
#define PIN_LORA_SCK         18
#define PIN_LORA_CS          5
#define PIN_LORA_RST         2
#define PIN_LORA_DIO1        4
#define PIN_LORA_DIO2        14

#define PIN_I2C_SDA          21
#define PIN_I2C_SCL          22

#define PIN_SONAR_TRIG       17 //TRIG/SCL/RX
#define PIN_SONAR_ECHO       16 //ECHO/SDA/TX

#define PIN_RGB1             13
#define PIN_RGB2             15

#define PIN_TX1              25
#define PIN_RX1              26

#define PIN_OUT1             32
#define PIN_OUT2             33

#define PIN_BTN              0

#define PIN_GPIO             27


#define CORE                 1
#define SIMPLE_SIZE          10

#define AP_NAME              "SVETOFOR_BOX_10"
#define AP_PASS              ""


#define DEFAULT_SENSOR_INSTALL_TYPE INSTALL_TYPE_NORMAL
#define DEFAULT_SENSOR_GROUND       1500
#define DEFAULT_SENSOR_LIMIT        250
#define DEFAULT_SENSOR_MIN_DIST     1250
#define DEFAULT_SENSOR_MAX_DIST     1750
#define COLOR_NONE             0x000000    //Цвет "никакой" (черный)
#define COLOR_FREE1            0x0000FF    //Цвет "свободно" №1
#define COLOR_FREE2            0x00FF00    //Цвет "свободно" №2
#define COLOR_FREE_DEFAULT     COLOR_FREE1 //Цвет "свободно" по умолчанию
#define COLOR_BLINK1           0x7F7F7F    //Цвет "свободно мигание" (если включен) №1
#define COLOR_BLINK2           0xFF007F    //Цвет "свободно мигание" (если включен) №2
#define COLOR_BLINK_DEFAULT    COLOR_BLINK1//Цвет "свободно мигание" по умолчанию
#define COLOR_BUSY1            0xFF0000    //Цвет "занято" №1
#define COLOR_BUSY2            0x000000    //Цвет "занято" №2
#define COLOR_BUSY_DEFAULT     COLOR_BUSY1 //Цвет "занято" по умолчанию
#define COLOR_NAN              0xFF007F    //Цвет "NAN"
#define COLOR_GROUND           0xA5FF00    //Цвет "установка земли"
#define COLOR_SAVE             0xFFFFFF    //Цвет "сохранение"
#define COLOR_ERROR            0xFF7F00    //Цвет "ошибка"

#define COLOR_WIFI_NONE        0x000000    //Цвет "WiFi не конфигуен"
#define COLOR_WIFI_OFF         0xFF0000    //Цвет "WiFi не подключен"
#define COLOR_WIFI_ON          0x00FF00    //Цвет "WiFi подключен"
#define COLOR_WIFI_WAIT        0x0FFF00    //Цвет "WiFi попытка подключения"
#define COLOR_WIFI_AP          0x00FF7F    //Цвет "Режим точки доступа"
#define COLOR_WIFI_AP1         0xFFFFFF    //Цвет "Точка доступа всегда включена"

#define DEFAULT_NAN_VALUE_FLAG NAN_VALUE_IGNORE

#define DEVICE_NAME            "3139_SVETOFORBOX.RU_192.168.4.1"
#define DEVICE_PASS0           "superadmin"
#define DEVICE_PASS1           "admin"
#define SENSOR_GROUND_STATE    false
#define COUNT_RGB1             50   //Число светодиодов

#endif
