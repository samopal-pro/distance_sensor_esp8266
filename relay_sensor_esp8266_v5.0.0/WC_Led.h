/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#ifndef WS_LED_h
#define WS_LED_h
#include <arduino.h>
#include "src/Adafruit_NeoPixel.h"  // https://github.com/adafruit/Adafruit_NeoPixel
#include "Config.h"

#define LED_TM_DEFAULT     500 //Залержка между эффектами "По умолчанию"
#define WLED_MODE_DEFAULT   1   //Режим "По умолчанию"
#define LED_MAX_BRIGHTNESS 255 //Максимальная яркость (0-255)

enum T_LED_WIFI_MODE {
   LED_WIFI_NONE    = 0,  
   LED_WIFI_OFF     = 1,
   LED_WIFI_ON      = 2,
   LED_WIFI_WAIT    = 3,
   LED_WIFI_AP      = 4,
   LED_WIFI_AP1     = 5
};

enum T_LED_BASE_MODE {
   LED_BASE_NONE    = 0,
   LED_BASE_FREE    = 1,
   LED_BASE_BUSY    = 2,
   LED_BASE_NAN     = 3,
   LED_BASE_GROUND  = 4,
   LED_BASE_SAVE    = 5
};



extern uint8_t ws_mode;
extern int      ws_mode_save;
extern uint16_t ws_tm;
extern int      ws_stat;
//extern  RgbColor ws_color, ws_color_save;
extern bool ws_enable;

extern Adafruit_NeoPixel *strip;

void ledInit();
//void ledLoop();
void ledSetWiFiMode(T_LED_WIFI_MODE _mode);
void ledSetBaseMode(T_LED_BASE_MODE _mode, bool _saveFlag = false);
void ledRestoreMode();
//void ledSetExtMode(T_LED_EXT_MODE _mode);
//void setColor(uint32_t color);


#endif
