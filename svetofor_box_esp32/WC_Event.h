/**
* Проект контроллера автомоек. Версия 10 от 2025
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#ifndef WS_EVENT_h
#define WS_EVENT_h
#include <arduino.h>
#include "MyConfig.h"
#include "src/DFPlayer/DFRobotDFPlayerMini.h"
#include "src/NeoPixel/Adafruit_NeoPixel.h"    // https://github.com/adafruit/Adafruit_NeoPixel


#define MAX_RGB_STACK_ITEMS    10
#define TIMER_MP3              500
#define DEFAULT_TIMER_MP3      300000
#define DEFAULT_DELAY_MP3      0


#define PRIORITY_MP3_MINIMAL   0
#define PRIORITY_MP3_MEDIUM    1
#define PRIORITY_MP3_HIGH      2
#define PRIORITY_MP3_MAXIMAL   3
#define PRIORITY_MP3_75        4

#define DAFAULT_PRIORITY_MP3   PRIORITY_MP3_MINIMAL



enum TEVENT_TYPE_t {
  ET_DISABLE    = 0, //CСобытие отключено
  ET_NORMAL     = 1, //Событие работает на ON/OFF
  ET_PULSE      = 2, //Событие выдает имульс на заданное время при включении или выключении
  ET_PULSE_OFF  = 3, //Событие выдает имульс на заданное время (не используется)
  ET_PWM        = 4, //Событие выдает имульсы с заданной длительностью и паузой
  ET_PULSE2     = 5  //Событие выдает имульс на заданное время при включение и выключении
   
};

enum TEVENT_RGB_TYPE_t {
  ERT_NONE      = 0, //Cобытие отключено
  ERT_NORMAL    = 1, //Горит постоянно
  ERT_BLINK     = 2, //Мигает с частотой DelayOn, DelauOff
  ERT_MP3       = 3, //Мигает цветом ColorMP3 и частотой TIMER_MP3
  ERT_RAINBOW   = 4 //Проигрывает радугу   
};

enum TEVENT_STATUS_t {
   ES_NONE     = 0,
   ES_WAIT_ON  = 1,
   ES_ON       = 2,
   ES_WAIT_OFF = 3,
   ES_OFF      = 4
};

typedef struct {
   int ID;
   TEVENT_TYPE_t Type;
   uint32_t TimeOn;
   uint32_t TimeOff;
   uint32_t Color1;
   uint32_t Color2;
   bool Flag; 
}RGB_STACK_ITEM_t;

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

class TEventRGB{
public:
    TEventRGB( uint8_t _pin, int _num, int _first );
    void set(uint32_t _color1, uint32_t _color2, uint32_t _color11=COLOR_NONE, uint32_t _color12=COLOR_NONE, uint32_t _timer_on=0, uint32_t _timer_off=0 );
    void setColor( uint32_t _color1, uint32_t _color2 );
    void setColor0( uint32_t _color );
    void setColor1( uint32_t _color );
    void setMP3( uint32_t _color );
    void setRainbow( bool _flag );
    void setBrightness( int br);
    void copyTo(TEventRGB *dist);
    void set(TEventRGB *src);
    TEVENT_RGB_TYPE_t  loop();
    TEVENT_STATUS_t State;
    bool isRainbow;
    bool isMP3;
    uint32_t Color1;
    uint32_t Color2; 
    uint32_t ColorBlink1;
    uint32_t ColorBlink2;
    uint32_t ColorMP3;
    uint32_t TimerOn;
    uint32_t TimerOff;
private:
    Adafruit_NeoPixel *Strip;
    int      Num;
    int      First;
    uint32_t msOn;
    uint32_t msOff;

    int      IncRainbow;
    int      HueRainbow;
};

class TEventMP3 {
private:
    DFRobotDFPlayerMini *Player;
    bool isPlayer;
    bool isLoop;
    Stream *SerialPlayer;
    void init();
    int Dir;
    int Track;

    int Priority;

    uint32_t msOn;
    uint32_t msOff;
    uint32_t ms1;
    bool waitPlayer;

    bool isOn;
    void _start();
    void _stop();
    void _replay(uint32_t _delay);


public:
    uint32_t DelayOn;
    uint32_t TimerOn;

    TEvent *ColorEvent;
    uint32_t Color;
    uint32_t ColorTimer;
    TEVENT_STATUS_t State;
    TEventMP3(Stream &_stream,  void (*_handle)(bool) = NULL );
    void setVolume(int _volume);
    void setColor(uint32_t _color=COLOR_NONE, uint32_t _timer=0);


    void start(int _dir, int _track, int _priority = DAFAULT_PRIORITY_MP3, uint32_t _delay = DEFAULT_DELAY_MP3, uint32_t _timer = DEFAULT_TIMER_MP3, bool _is_wait = true, bool _loop=false );
    
    void stop();
    TEVENT_STATUS_t loop();

    void (*Handle)(bool);
};

/*

class TSaveRGB {
   public:
      TSaveRGB( TEventRGB *_event, int _id = 1);
      int Save(int _id, TEVENT_TYPE_t _type, uint32_t _timeOn, uint32_t _timeOff, uint32_t _color1, uint32_t _color2 );
      int Restore( int _id );
      void Clear();
   private:
      TEventRGB *EventRGB;
      int Count;
      int ID;
      RGB_STACK_ITEM_t StackRGB[MAX_RGB_STACK_ITEMS];
      int Push(int _id );
      int Pop();
};
*/
#endif
