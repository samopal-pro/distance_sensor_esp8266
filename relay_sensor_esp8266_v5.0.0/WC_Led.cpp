/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#include "WC_Led.h"




uint8_t ws_mode   = 0;
int      ws_mode_save  = -1;
//uint16_t ws_tm    = WS_TM_DEFAULT;
int      ws_stat  = 0;
//strip.Color ws_color = strip.Color(0, 0, 0), ws_color_save;
bool ws_enable = true;

Adafruit_NeoPixel *strip;
T_LED_WIFI_MODE ledWiFiMode = LED_WIFI_NONE;
T_LED_BASE_MODE ledBaseMode = LED_BASE_NONE, ledBaseModeSave = LED_BASE_NONE;
//T_LED_EXT_MODE ledExtMode   = LED_EXT_NONE;
bool changeWiFiMode         = true;
bool changeMode             = true;
//bool changeExtMode          = false;
bool ledFlag                = true;
int loopCount = 0;


/**
* Инициализация светодиодной ленты
*/
void ledInit(){
   if( PinLed < 0 )return;
   strip = new Adafruit_NeoPixel(LED_COUNT, PinLed, NEO_GRB + NEO_KHZ800);
   strip->begin();

   uint16_t _hue = 0;
   uint16_t _inc = 65536/LED_COUNT;
   for( int i=0; i<40; i++){
      for( int j=0; j<LED_COUNT; j++){
         uint16_t h = _hue + _inc*j;
         strip->setPixelColor(j, strip->ColorHSV(h, 255, 100));     
      }
      strip->show();
      _hue += _inc;
//      Serial.println(_hue);
      delay(50);
   }
   ledSetWiFiMode(LED_WIFI_OFF);
   ledSetBaseMode(LED_BASE_NONE);
}


void ledSetWiFiMode(T_LED_WIFI_MODE _mode){
   if( PinLed < 0 )return;
   if( _mode == ledWiFiMode )return;
   ledWiFiMode    = _mode;
   switch(ledWiFiMode){
      case LED_WIFI_NONE: strip->setPixelColor(0,COLOR_WIFI_NONE);break;
      case LED_WIFI_OFF:  strip->setPixelColor(0,COLOR_WIFI_OFF);break;
      case LED_WIFI_ON:   strip->setPixelColor(0,COLOR_WIFI_ON);break;
      case LED_WIFI_WAIT: strip->setPixelColor(0,COLOR_WIFI_WAIT);break;
      case LED_WIFI_AP:   strip->setPixelColor(0,COLOR_WIFI_AP);break;
      case LED_WIFI_AP1:  strip->setPixelColor(0,COLOR_WIFI_AP1);break;    
      }      
   strip->show();
   changeWiFiMode = true;
   Serial.printf("Led wifi mode %d\n", ledWiFiMode);
}

void ledSetBaseMode(T_LED_BASE_MODE _mode, bool _saveFlag ){
   if( PinLed < 0 )return;
   if( EA_Config.Brightness <= 10 )strip->setBrightness(EA_Config.Brightness*25);
   else strip->setBrightness(255);
//   Serial.printf("1 Set led mode %d\n",ledBaseMode);
   if( _saveFlag )ledBaseModeSave = ledBaseMode;
   if( _mode == ledBaseMode )return;
   ledBaseMode    = _mode;
   switch(ledBaseMode){
      case LED_BASE_NONE:     ledSetColor(COLOR_NONE);break;
      case LED_BASE_FREE:     ledSetColor(EA_Config.ColorFree);break;
      case LED_BASE_BUSY:     ledSetColor(EA_Config.ColorBusy);break;
      case LED_BASE_NAN:      ledSetColor2(0,COLOR_NAN);break;
      case LED_BASE_NAN_FREE: ledSetColor2(EA_Config.ColorFree,COLOR_NAN);break;
      case LED_BASE_NAN_BUSY: ledSetColor2(EA_Config.ColorBusy,COLOR_NAN);break;
      case LED_BASE_GROUND:   ledSetColor(COLOR_GROUND);break;
      case LED_BASE_SAVE:     ledSetColor(COLOR_SAVE);break;
      case LED_BASE_ERROR:    ledSetColor(COLOR_ERROR);break;
   }
   
//   Serial.printf("2 Set led mode %d\n",ledBaseMode);
   changeMode = true;
}

void ledSetColor(uint32_t _color){
   if( PinLed < 0 )return;
   for( int i=1; i<LED_COUNT; i++)strip->setPixelColor(i,_color);
   strip->show();
}

void ledSetColor2(uint32_t _color1, uint32_t _color2){
   if( PinLed < 0 )return;
   for( int i=1; i<LED_COUNT; i++)
      if( i%2 ){
         if( _color1 > 0 )strip->setPixelColor(i,_color1); 
      }
      else strip->setPixelColor(i,_color2);
   strip->show();
}

void ledRestoreMode(){ ledSetBaseMode(ledBaseModeSave); }

/*
void ledLoop(){
   if( PinLed < 0 )return;
   if( changeMode == false && changeWiFiMode == false && changeExtMode == false )return;
   if( ledFlag == true )ledFlag = false;
   else ledFlag = true;
   if( changeWiFiMode ){
      changeWiFiMode = false;
      switch(ledWiFiMode){
         case LED_WIFI_NONE: strip->setPixelColor(0,strip->Color(0,0,0));break;
         case LED_WIFI_OFF:  strip->setPixelColor(0,strip->Color(255,0,0));break;
         case LED_WIFI_ON:   strip->setPixelColor(0,strip->Color(0,255,0));break;
         case LED_WIFI_WAIT: strip->setPixelColor(0,strip->Color(127,127,0));break;
         case LED_WIFI_AP:   
            if(ledFlag)strip->setPixelColor(0,strip->Color(255,255,255));
            else strip->setPixelColor(0,strip->Color(0,0,0));
            changeWiFiMode = true;
            break;
         case LED_WIFI_AP1:  strip->setPixelColor(0,strip->Color(255,255,255));break;
      }      
   }
   if( changeExtMode){
       if( loopCount <= 0){
           changeMode    = true;
           changeExtMode = false;
           ledExtMode    = LED_EXT_NONE;
       }
       else {
           changeMode = false;
           switch(ledExtMode){
               case LED_EXT_NAN :
                  for( int i=1; i<LED_COUNT; i++)if( i%2 )strip->setPixelColor(i,strip->Color(255,0,255));
                  break;
               case LED_EXT_GROUND :
                  for( int i=1; i<LED_COUNT; i++)
                     if(ledFlag)strip->setPixelColor(i,strip->Color(127,127,0));
                     else strip->setPixelColor(i,strip->Color(0,0,0));
                  break;
               case LED_EXT_BTN3 :
               case LED_EXT_BTN10 :
                  for( int i=1; i<LED_COUNT; i++)
                     if(ledFlag)strip->setPixelColor(i,strip->Color(127,127,0));
                     else strip->setPixelColor(i,strip->Color(0,0,0));
                  break;
                 
           }
           loopCount--;
       }
   }
   if( changeMode ){
     changeMode = false;
     uint32_t color = 0;
      switch(ledBaseMode){
         case LED_BASE_NONE: color = strip->Color(0,0,0);break;
         case LED_BASE_FREE: color = strip->Color(0,0,255);break;
         case LED_BASE_BUSY: color = strip->Color(255,0,0);break;
 //        case LED_BASE_CALIBRATE: color = strip->Color(127,237,0);break;
      }
      for( int i=1; i<LED_COUNT; i++)strip->setPixelColor(i,color);
   }
   strip->show();
   
}

void setColor(uint32_t color){
   if( PinLed < 0 )return;
   for( int i=1; i<LED_COUNT; i++)strip->setPixelColor(i,color);  
   changeMode = true;
   strip->show();
}
*/

/*
void WS_set( int mode ){
  if( WS_PIN < 0 )return;
// Если режим не поменялся  
  if( mode == ws_mode )return;
  ws_mode_save = ws_mode;
  ws_mode = mode;
//  Serial.printf("WS: mode=%d\n",ws_mode);
   switch( ws_mode ){
     case 1: //Мигнуть белым, синим, красным и вернуться к старому режиму
        for( int i=0; i<WS_PixelCount; i++ ){
           if( i%3 == 0 )strip.setPixelColor(i,strip.Color(127, 127, 127));
           else if( i%3 == 1 )strip.setPixelColor(i,strip.Color(0, 0, 255));
           else strip.setPixelColor(i,strip.Color(255, 0, 0));
        }
        strip.show();
        break;
      case 2: //Мигнуть пять раз и уйти в старый режим (настройка земли)
         for( int j=0; j<5; j++ ){
            WS_set(0,0,0);
            delay(200);
            WS_set(255,255,255); 
            delay(200);
         }
         WS_set(ws_mode_save);
         break;
       case 3: //Зажечь красный цвет (мойка занята)
         WS_set(255,0,0);            
         break;
      case 4: //Горит синим цветом (мойка свободна)
         WS_set(0,0,255);            
         break;
      case 12: //Мигнуть один раз желтым цветом и врнеуться к предыдущему (нет RTC)
         for( int j=0; j<1; j++ ){
            WS_set(0,0,0);
            delay(200);
            WS_set(255,255,0); 
            delay(200);
         }
         WS_set(ws_mode_save);
         break;
      default: 
         WS_set(0, 0, 0);
   }
  
}

void WS_set( uint8_t R, uint8_t G, uint8_t B,bool is_first){
  if( WS_PIN < 0 )return;
  if( is_first ){
     strip.setPixelColor(0,strip.Color(R,G,B));  
  }
  else {
    for( int i=1; i<WS_PixelCount; i++) strip.setPixelColor(i,strip.Color(R,G,B));
  }
  strip.show();    
}

*/