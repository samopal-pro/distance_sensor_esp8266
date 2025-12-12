#include "SBTN.h"

uint16_t _count_btn = 0; // Общий счетчик кнопок

//***********************************************************************************************************************
// Методы класса для  кнопки упоавляемой setBtnState()
//***********************************************************************************************************************


//***********************************************************************************************************************
// Методы класса для аппаратной кнопки подключенной к дискретному входу
//***********************************************************************************************************************

/**
* Конструктор класса SBTN
* @param pin - пин дискреиного входа
* @param _bounce - таймаут на подавление дребезга контактов (мс). По умолчанию 250
* @param _press_state - состояние входа при нажатой кнопке. LOW - низкий уровень (по умолчанию), HIGH - высокий
*/
SBTN::SBTN(uint8_t pin, uint32_t _bounce, bool _press_state): SBTN_base( NULL, _bounce ){
   Pin = pin;
   PressState = _press_state;
   if( PressState == LOW )pinMode(Pin, INPUT_PULLUP);
   else pinMode(Pin, INPUT);
}

/**
* Метод определения нажатой кнопки
* @return true если кнопка нажата
*/
bool SBTN::isPress(void){ 
   if( PressState == LOW )return !digitalRead(Pin); 
   else return digitalRead(Pin);
}

//***********************************************************************************************************************
// Методы класса виртуальной кнопки, базавого для остальных классов
//***********************************************************************************************************************

/**
* Конструктор класса SBTN
* @param func - указатель на функцию обработчика нажатия кнопки
* @param _bounce - таймаут на подавление дребезга контактов (мс). По умолчанию 250
*/
SBTN_base::SBTN_base(bool (*func)(void), uint32_t _bounce){
   isPressPtr  = func;  
   ms_press    = 0;
   is_press    = false;
   tm_bounce   = _bounce;
   number_btn  = _count_btn++;
   count_event = 0;
   is_debug    = false;
   is_timer    = false;
   timer_press = 0;
   timer_reset_count_event = 0;
}

/**
* Обработчик нажатия кнопки с использовнием указателя на функцию
* @return true если кнопка нажата
*/
bool SBTN_base::isPress(void){
  if( isPressPtr == NULL )return false;
  return (*isPressPtr)();
}

/**
* Функция запускаемая в бесконечном цикле
* Вызывает функции оратного вызова по событиям и возвращает состояние нажатой кнопки
*/
SBTN_EVENT_t SBTN_base::loop(){
   bool _press_flag = isPress(); 
   uint32_t _ms = millis();
   ms_delta = _ms - ms_press; //Компенсируем переполнение тсистемного счетчика миллисекунд
   if( ms_press > _ms )ms_delta = 0 - ms_press + _ms; 
   if( _press_flag ){ //Фиксируем событие нажатой кнопки
      if( !is_press ){ //Фиксируем первое событие нажатой кнопки
         is_press = true;
         is_timer = true;
         ms_press = _ms;     
         count_event++;
         if( is_debug ){
            Serial.print(F("!!! Press key="));
            Serial.print(number_btn);
            Serial.print(F(" event="));
            Serial.print(count_event);
            Serial.println();
         }

         if( PressPtr != NULL )PressPtr(number_btn,count_event);   
         return SB_PRESS;
      }
     if( is_press && timer_press>0 && is_timer && timer_press<ms_delta ){
         is_timer = false;
         if( is_debug ){
            Serial.print(F("!!! Timer key="));
            Serial.print(number_btn);
            Serial.print(F(" event="));
            Serial.print(count_event);
            Serial.println();
         }

         if( PressPtr != NULL )TimerPtr(number_btn,count_event);   
         return SB_TIMER;

     }
   }
   else {  //Фиксируем событие отжатой кнопки
      if( is_press && ( ms_delta >= tm_bounce  ) ){ //Фиксируем событие отжатой кнопки по истесении интервала дребезга контактов
         is_press = false;
         if( is_debug ){
            Serial.print(F("!!! Release key="));
            Serial.print(number_btn);
            Serial.print(F(" event="));
            Serial.print(count_event);
            Serial.print(F(" time="));
            Serial.print(ms_delta);
            Serial.println(F(" ms"));
         }
         if( ReleasePtr != NULL )ReleasePtr(number_btn,count_event,ms_delta);   
//         ms_press = 0;    
//	     count_event = 0;
         return SB_RELEASE;    
      }
   }
   if( ms_delta > timer_reset_count_event && count_event > 0 ){
	  count_event = 0;
	  return SB_TIMER_COUNT;
   }
   return SB_NONE;
}

/**
* Установить таймаут подавоения дребезга контактов, мс
*/
void SBTN_base::setBounceTM(uint32_t _tm){ tm_bounce = _tm; }

/**
* Установить режим выдочи сообщений по событиям на экран
*/
void SBTN_base::setDebug(bool flag ){ is_debug = flag; }

/**
* Вернуть количество событий нажатия кнопки
*/
uint16_t SBTN_base::getCountEvent(void){ return count_event; }

/**
* Вернуть номер кнопки
*/
uint16_t SBTN_base::getNumberBtn(void){ return number_btn; }

/**
* Вернуть время нажатия кнопки
*/
uint32_t SBTN_base::getPressTime(void){ return ms_delta; }

/**
* Установить таймер удержания кнопки
*/
void SBTN_base::setTimer(uint32_t _tm){ timer_press = _tm; }

/**
* Установить таймер сброса счетчика нажатий кнопки
*/
void SBTN_base::setTimerEventCount(uint32_t _tm){ timer_reset_count_event = _tm; }

