/**
* Проект контроллера автомоек. Версия 10 от 2025
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#ifndef WS_EVENT_h
#define WS_EVENT_h
#include <arduino.h>
#include "MyConfig.h"


enum TEVENT_TYPE_t {
  ET_DISABLE    = 0, //CСобытие отключено
  ET_NORMAL     = 1, //Событие работает на ON/OFF
  ET_PULSE      = 2, //Событие выдает имульс на заданное время при включении или выключении
  ET_PULSE_OFF  = 3, //Событие выдает имульс на заданное время (не используется)
  ET_PWM        = 4, //Событие выдает имульсы с заданной длительностью и паузой
  ET_PULSE2     = 5  //Событие выдает имульс на заданное время при включение и выключении
   
};

enum TEVENT_STATUS_t {
   ES_NONE     = 0,
   ES_WAIT_ON  = 1,
   ES_ON       = 2,
   ES_WAIT_OFF = 3,
   ES_OFF      = 4
};


class TEvent {
public:
   TEvent(uint32_t _delayOn = 0, uint32_t _delayOff = 0, void (*_handle)(bool) = NULL);
   void setType( TEVENT_TYPE_t _type, uint32_t _timeOn = 0, uint32_t _timeOff = 0 );
   void on();
   void on( uint32_t _delay );
   void off();
   void off( uint32_t _delay );
   void copyTo(TEvent *dist);

   void reset();
   bool changeState();
   TEVENT_STATUS_t loop();
   TEVENT_TYPE_t Type;
   TEVENT_STATUS_t State;
   TEVENT_STATUS_t SaveState;
   bool isChangeState;
   uint32_t DelayOn;
   uint32_t DelayOff;
   uint32_t TimeOn;
   uint32_t TimeOff;
   void (*Handle)(bool);
private:
   
   uint32_t msOn;
   uint32_t msOff;
   
//   void (*HandleOff)(void);
//   TEvent *EventOn;
 //  TEvent *EventOff;

   bool isOn;
   void setOn();
   void setOff();
};

class TEventRGB: public TEvent {
   public:
       TEventRGB(uint32_t _delayOn = 0, uint32_t _delayOff = 0, void (*_handle)(bool) = NULL, uint32_t _color1 = COLOR_BLACK, uint32_t _color2 = COLOR_NONE );
       void setColor(uint32_t _color1, uint32_t _color2);
       uint32_t Color1;
       uint32_t Color2; 
   void copyTo(TEventRGB *dist);
};

class TEventMP3: public TEvent {
   public:
       TEventMP3(uint32_t _delayOn = 0, uint32_t _delayOff = 0, void (*_handle)(bool) = NULL, int _dir = 1, int _sound = 1, bool _loop = false );
       void setSound(int _dir, int _sound, bool _loop = false);
       int Dir;
       int Sound; 
       bool Loop;
   void save(TEventMP3 *dist);
   void copyTo(TEventMP3 *src);
};

#endif
