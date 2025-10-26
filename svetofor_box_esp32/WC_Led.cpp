/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#include "WC_Led.h"






Adafruit_NeoPixel *strip, *strip2;
uint32_t saveColor = 0, saveColor2 = 0;


/**
* Инициализация светодиодной ленты
*/
void ledInit(){
   strip = new Adafruit_NeoPixel(COUNT_RGB1, PIN_RGB1, NEO_GRB + NEO_KHZ800);
   strip->begin();
   strip2 = new Adafruit_NeoPixel(COUNT_RGB2, PIN_RGB2, NEO_GRB + NEO_KHZ800);
   strip2->begin();

   uint16_t _hue = 0;
   uint16_t _inc = 65536/COUNT_RGB1;
   for( int i=0; i<40; i++){
      for( int j=0; j<COUNT_RGB1; j++){
         uint16_t h = _hue + _inc*j;
         strip->setPixelColor(j, strip->ColorHSV(h, 255, 100));     
         strip2->setPixelColor(j, strip2->ColorHSV(h, 255, 100));     
      }
      strip->show();
      strip2->show();
      _hue += _inc;
//      Serial.println(_hue);
      delay(50);
   }
   ledSetColor0(COLOR_WIFI_NONE);
   led2SetColor0(COLOR_WIFI_NONE);
// Чтобы до первого нормального измерения лета горела малиновым
   ledSetColor(COLOR_NAN);
   led2SetColor(COLOR_NAN);
}

void ledSetColor0(uint32_t _color){
   strip->setPixelColor(0,_color);
   strip->show();
}


void ledSetColor(uint32_t _color, bool _flag){
   if(_flag)saveColor = _color;
   for( int i=1; i<COUNT_RGB1; i++)strip->setPixelColor(i,_color);
   strip->show();
}

void ledSetColor2(uint32_t _color1, uint32_t _color2, bool _flag){
   if(_flag)saveColor = _color2;
   for( int i=1; i<COUNT_RGB1; i++)
      if( i%2 ){
         if( _color1 != 0xFFFFFFFF ) strip->setPixelColor(i,_color1); 
      }
      else strip->setPixelColor(i,_color2);
   strip->show();
}

void ledRestoreColor(){
   ledSetColor(saveColor);
}

/********************************************************************************************************************************************************************************/
// RGB2
/********************************************************************************************************************************************************************************/
/**
* Инициализация светодиодной ленты
*/
void led2Init(){
   strip2 = new Adafruit_NeoPixel(COUNT_RGB1, PIN_RGB2, NEO_GRB + NEO_KHZ800);
   strip2->begin();

   uint16_t _hue = 0;
   uint16_t _inc = 65536/COUNT_RGB2;
   for( int i=0; i<40; i++){
      for( int j=0; j<COUNT_RGB2; j++){
         uint16_t h = _hue + _inc*j;
         strip2->setPixelColor(j, strip->ColorHSV(h, 255, 100));     
      }
      strip2->show();
      _hue += _inc;
//      Serial.println(_hue);
      delay(50);
   }
   led2SetColor0(COLOR_WIFI_NONE);
// Чтобы до первого нормального измерения лета горела малиновым
   led2SetColor(COLOR_NAN);
}

void led2SetColor0(uint32_t _color){
   strip2->setPixelColor(0,_color);
   strip2->show();
}


void led2SetColor(uint32_t _color, bool _flag){
   if(_flag)saveColor2 = _color;
   for( int i=1; i<COUNT_RGB2; i++)strip2->setPixelColor(i,_color);
   strip2->show();
}

void led2SetColor2(uint32_t _color1, uint32_t _color2, bool _flag){
   if(_flag)saveColor2 = _color2;
   for( int i=1; i<COUNT_RGB2; i++)
      if( i%2 ){
         if( _color1 != 0xFFFFFFFF ) strip2->setPixelColor(i,_color1); 
      }
      else strip2->setPixelColor(i,_color2);
   strip2->show();
}

void led2RestoreColor(){
   led2SetColor(saveColor);
}

