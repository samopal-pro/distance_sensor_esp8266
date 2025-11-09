#ifndef SBTN_h
#define SBTN_h
#include "Arduino.h"

typedef enum {
   SB_NONE        = 0,
   SB_PRESS       = 1,
   SB_RELEASE     = 2,
   SB_TIMER       = 3,
   SB_TIMER_COUNT = 4
} SBTN_EVENT_t;


/**
* Базовый класс для всех кнопок
*/
class SBTN_base {
   private:
      void (*PressPtr)(uint16_t, uint16_t) = NULL;
      void (*TimerPtr)(uint16_t, uint16_t) = NULL;
      void (*ReleasePtr)(uint16_t, uint16_t,uint32_t) = NULL;
      bool (*isPressPtr)(void) = NULL;
      uint32_t ms_press;
      bool is_press;
      bool is_timer;
      uint32_t tm_bounce;
      uint16_t number_btn;
      uint16_t count_event;
      uint32_t ms_delta;
      uint32_t timer_press;
	  uint32_t timer_reset_count_event;
      bool     is_debug;
   public:
      SBTN_base(bool (*func)(void), uint32_t _bounce=250);
      SBTN_EVENT_t loop();
      void setPressCallback( void (*func)(uint16_t, uint16_t) );
      void setReleaseCallback( void (*func)(uint16_t, uint16_t, uint32_t) );
      void setTimerCallback( void (*func)(uint16_t, uint16_t, uint32_t) );
      void setBounceTM(uint32_t _tm);
      void setDebug(bool flag = true);
      virtual bool isPress(void);
      uint16_t getCountEvent(void);
      uint16_t getNumberBtn(void);
      void setTimer(uint32_t _tm);
      uint32_t getPressTime(void);
	  void setTimerEventCount(uint32_t _tm);
};

/**
* Класс для аппаратной кнопки подключенной к дискретному вводу
* По умолчанию _press_stat = LOW - кнопка замыкается на землю, включена внутренняя подтяжка к VCC
* _press_stat = HIGH - кнопка замыкается на плюс (или внешний полоэительный импульс)
*/
class SBTN: public SBTN_base {
   private:
      uint8_t Pin;
      bool    PressState;
   public:
      SBTN(uint8_t _pin, uint32_t _bounce=250, bool _press_state=LOW );
      bool isPress(void);
};



#endif
