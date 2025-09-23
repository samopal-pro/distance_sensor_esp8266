/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#include "WC_Led.h"






Adafruit_NeoPixel *strip;
uint32_t saveColor = 0;


/**
* Инициализация светодиодной ленты
*/
void ledInit(){
   strip = new Adafruit_NeoPixel(COUNT_RGB1, PIN_RGB1, NEO_GRB + NEO_KHZ800);
   strip->begin();

   uint16_t _hue = 0;
   uint16_t _inc = 65536/COUNT_RGB1;
   for( int i=0; i<40; i++){
      for( int j=0; j<COUNT_RGB1; j++){
         uint16_t h = _hue + _inc*j;
         strip->setPixelColor(j, strip->ColorHSV(h, 255, 100));     
      }
      strip->show();
      _hue += _inc;
//      Serial.println(_hue);
      delay(50);
   }
   ledSetColor0(COLOR_WIFI_NONE);
// Чтобы до первого нормального измерения лета горела малиновым
   ledSetColor(COLOR_NAN);
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
         if( _color1 > 0 )strip->setPixelColor(i,_color1); 
      }
      else strip->setPixelColor(i,_color2);
   strip->show();
}

void ledRestoreColor(){
   ledSetColor(saveColor);
}

void setLedDistance( float _dist, bool _on){
  if( isnan(_dist) ){
      if(jsonConfig["RGB1"]["IS_NAN_MODE"].as<bool>())
//      Serial.println("!!! NAN NAN");
       switch(jsonConfig["RGB1"]["NAN_MODE"].as<int>()){
         case NAN_VALUE_IGNORE: ledSetColor2(COLOR_NONE,COLOR_NAN); break;
         case NAN_VALUE_BUSY:   ledSetColor2(COLOR_BUSY_DEFAULT,COLOR_NAN); break;
         case NAN_VALUE_FREE:   ledSetColor2(COLOR_FREE_DEFAULT,COLOR_NAN); break;
      } 
  }
  else {
     if( _on != SENSOR_GROUND_STATE )ledSetColor(COLOR_BUSY_DEFAULT);
     else ledSetColor(COLOR_FREE_DEFAULT);
  }
}

//void ledRestoreMode(){ ledSetBaseMode(ledBaseModeSave); }

