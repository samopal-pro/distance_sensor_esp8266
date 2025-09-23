/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#ifndef WS_LED_h
#define WS_LED_h
#include <arduino.h>
#include "src/NeoPixel/Adafruit_NeoPixel.h"  // https://github.com/adafruit/Adafruit_NeoPixel
#include "MyConfig.h"
#include "WC_Config.h"

#define LED_TM_DEFAULT     500 //Залержка между эффектами "По умолчанию"
#define WLED_MODE_DEFAULT   1   //Режим "По умолчанию"
#define LED_MAX_BRIGHTNESS 255 //Максимальная яркость (0-255)

extern float Distance;
extern bool SensorOn;  

extern Adafruit_NeoPixel *strip;

void ledInit();
//void ledLoop();
void ledRestoreMode();

void ledSetColor0(uint32_t _color);
void ledSetColor(uint32_t _color, bool _flag = true);
void ledSetColor2(uint32_t _color1, uint32_t _color2, bool _flag = true);
//void ledSetExtMode(T_LED_EXT_MODE _mode);
//void setColor(uint32_t color);
void setLedDistance( float _dist, bool _on);
void ledRestoreColor();
#endif
