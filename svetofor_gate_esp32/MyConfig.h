#ifndef MY_CONFIG
#define MY_CONFIG

// Выдача сообщение сенсоров 1- иницилизация и ошибки, 2 - измерение параметров
#define DEBUG_SENSORS        1
#define DEBUG_SERIAL

#define IS_LORA
//#define IS_NTP

#define SOFTWARE_V           "10.0.1"
#define HARDWARE_V           "10.0.1"
#define CONFIG_V             "10.0.3"

enum SENSOR_STAT_t {
  SS_NONE      = 0,
  SS_FREE      = 2,
  SS_BUSY      = 3,
  SS_NAN       = -1,
  SS_NAN_FREE  = -2,
  SS_NAN_BUSY  = -3,
  SS_RESTORE   = 100
};

#define PIN_LORA_MOSI        23
#define PIN_LORA_MISO        19
#define PIN_LORA_SCK         18
#define PIN_LORA_CS          5
#define PIN_LORA_RST         2
#define PIN_LORA_DIO1        4
#define PIN_LORA_DIO2        14
#define PIN_LORA_BUSY        -1

#define PIN_I2C_SDA          21
#define PIN_I2C_SCL          22

#define PIN_SONAR_TRIG       17 //TRIG/SCL/RX
#define PIN_SONAR_ECHO       16 //ECHO/SDA/TX

#define PIN_RGB1             13
#define PIN_RGB2             15

#define PIN_IR               PIN_RGB2

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
#define COLOR_NONE             0xFFFFFFFF  //Цвет прозрачный
#define COLOR_BLACK            0x000000    //Цвет "никакой" (черный)
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
#define COLOR_MP3_1            0x7F7F7F    //Цвет мигания во время произрывания звукового файла
#define COLOR_MP3_2            0xFFFF00    //Цвет мигания во время произрывания звукового файла
#define COLOR_MP3_DEFAULT      COLOR_MP3_1    //Цвет мигания во время произрывания звукового файла

#define COLOR_WIFI_NONE        0x000000    //Цвет "WiFi не конфигуен"
#define COLOR_WIFI_OFF         0xFF0000    //Цвет "WiFi не подключен"
#define COLOR_WIFI_ON          0x00FF00    //Цвет "WiFi подключен"
#define COLOR_WIFI_WAIT        0xFFFF00    //Цвет "WiFi попытка подключения"
#define COLOR_WIFI_AP          0x00FF7F    //Цвет "Режим точки доступа"
#define COLOR_WIFI_AP1         0xFFFFFF    //Цвет "Точка доступа всегда включена"

#define DEFAULT_NAN_VALUE_FLAG NAN_VALUE_IGNORE

#define DEVICE_NAME            "2025_SVETOFORBOX.RU_192.168.4.1"
#define DEVICE_PASSS           "svetoforbox"
#define DEVICE_PASS0           "superadmin"
#define DEVICE_PASS1           "admin"
#define SENSOR_GROUND_STATE    SS_FREE;
#define COUNT_RGB1             50   //Число светодиодов
#define COUNT_RGB2             50   //Число светодиодов

#define HTTP_PATH              "/api/1/sensor/99/"

#define MP3_BASE_DIR           2
#define MP3_SYSTEM_FULL_DIR    3
#define MP3_SYSTEM_SHORT_DIR   4

#define TB_HOST                "109.172.115.70"
#define TB_PORT                8088

#define TB_TOKEN               ""

#define TB_PROVISION_KEY       "_Svetofor10boxGate_key"
#define TB_PROVISION_SECRET    "_Svetofor10boxGate_secret"

#define TB_PROVISION_USER      "provision"

#define MQTT_WAIT_TM           20000

#define MAX_NUM_PARKING_SPACE  87
#endif
