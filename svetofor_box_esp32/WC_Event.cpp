/**
* Проект контроллера автомоек. Версия 10 от 2025
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/
#include "WC_Event.h"

/**
* Конструктор класса TEvent
* @param _delayOn   - Задержка при включении события, мс
* @param _delayOff  - Задержка при выключении события, мс
* @param _handleOn  - Функция вызываемая при включении события
* @param _hansleOff - Функция вызываемая при отключении события
*/
TEvent::TEvent(uint32_t _delayOn, uint32_t _delayOff, void (*_handle)(bool) ){
    DelayOn   = _delayOn;
    DelayOff  = _delayOff;
    Handle    = _handle;
//    HandleOff = _handleOff;
//    EventOn   = _eventOn;
//    EventOff  = _eventOff;
    State     = ES_NONE;
    SaveState = ES_NONE;
    Type      = ET_NORMAL;
    TimeOn    = 0;
    TimeOff   = 0;
    isOn      = false;
    isChangeState = false;
}

/**
* Изменение типа события
* @param _type      - Установка типа события
* @param _timeOn    - Длительность импульса события, мс (если PULSE, PILSE_OFF, PULSE2, PWM)
* @param _timeOff   - Длительность между импульсами, мс (если PWM)
*/
void TEvent::setType( TEVENT_TYPE_t _type, uint32_t _timeOn, uint32_t _timeOff ){
   Type       = _type;
   TimeOn     = _timeOn;
   TimeOff    = _timeOff;
/*
   Serial.print("!!!!! Relay2 type=");
   Serial.print(Type); 
   Serial.print(" t1=");
   Serial.print(TimeOn);
   Serial.print(" t2=");
   Serial.println(TimeOff);
*/
}


bool TEvent::changeState(){
   bool ret = isChangeState;
   isChangeState = false;
   return ret;
}

/**
* Включение события
*/
void TEvent::on(){
    if( State == ES_WAIT_ON || State == ES_ON )return;
    msOn = millis();
    if( DelayOn == 0 ){
       State = ES_ON;
       setOn();
    }
    else {
       State = ES_WAIT_ON;
    }
}

void TEvent::on( uint32_t _delay ){
   DelayOn = _delay;
   on();
}

/*
* Выключение события
*/
void TEvent::off(){
    if( State == ES_WAIT_OFF || State == ES_OFF )return;
    msOff = millis();
    if( DelayOff == 0 ){
       State = ES_OFF;
       setOff();
    }
    else {
       State = ES_WAIT_OFF;
    }
}

void TEvent::off( uint32_t _delay ){
   DelayOff = _delay;
   off();
}


void TEvent::reset(){
    State = ES_NONE;
}


void TEvent::setOn(){
   if( Handle != NULL )Handle(true);
   isOn = true;
}

void TEvent::setOff(){
   if( Handle != NULL && isOn )Handle(false);
   isOn = false;
}



/*
* Метод loop()
*/
TEVENT_STATUS_t TEvent::loop(){
   uint32_t _ms = millis();
   switch( State ){
       case ES_WAIT_ON :
          if( _ms - msOn > DelayOn ){
             State = ES_ON;
             msOn  = _ms;
             setOn();
          }
          break;  
       case ES_WAIT_OFF :   
          if( _ms - msOff > DelayOff ){
             State = ES_OFF;
             msOff  = _ms;
             setOff();
          }
          break; 
       case ES_ON :
          if( ( Type == ET_PULSE || Type == ET_PULSE2 || Type == ET_PWM )&&isOn&&( _ms - msOn > TimeOn)  ){
             msOn = _ms;
             setOff();
          }
          else if( (Type == ET_PWM )&&isOn==false&&( _ms - msOn > TimeOff) ){
             msOn = _ms;
             setOn();
          }
          break;
       case ES_OFF :
          if( ( Type == ET_PULSE_OFF || Type == ET_PULSE2  )&&!isOn  ){
             msOn = _ms;
             setOn();
          }      
          else if( ( Type == ET_PULSE_OFF || Type == ET_PULSE2 )&&isOn&&( _ms - msOn > TimeOn)  ){
             msOn = _ms;
             setOff();
          }
          break;
   }
   if( SaveState != State ){
       isChangeState = true;
       SaveState     = State;
   }
   return State;
 // Serial.printf("!!! Type=%d State=%d\n",(int)Type,(int)State);
}

void TEvent::copyTo(TEvent *dist){
    dist->Type     = Type;
    dist->DelayOn  = DelayOn;
    dist->DelayOff = DelayOff;
    dist->TimeOn   = TimeOn;
    dist->TimeOff  = TimeOff;
    dist->Handle   = Handle;
}


//***********************************************************************************************************************************************************************************
// Класс TEventRGB
//***********************************************************************************************************************************************************************************
TEventRGB::TEventRGB(uint32_t _delayOn , uint32_t _delayOff, void (*_handle)(bool), uint32_t _color1, uint32_t _color2 ):
   TEvent( _delayOn, _delayOff, _handle ){
    setColor(_color1, _color2);
}


void TEventRGB::setColor(uint32_t _color1, uint32_t _color2){
    Color1 = _color1;
    Color2 = _color2;
}

void TEventRGB::copyTo(TEventRGB *dist){
    TEvent::copyTo(dist);
    dist->setColor(Color1,Color2);
}

//***********************************************************************************************************************************************************************************
// Класс TEventMP3
//***********************************************************************************************************************************************************************************
TEventMP3::TEventMP3(uint32_t _delayOn , uint32_t _delayOff, void (*_handle)(bool), int _dir, int _sound, bool _loop ):
   TEvent( _delayOn, _delayOff, _handle ){
    setSound(_dir, _sound, _loop);
}


void TEventMP3::setSound(int _dir, int _sound, bool _loop){
    Dir    = _dir;
    Sound  = _sound;
    Loop   = _loop;
    Serial.printf("!!! Set MP3 %d %d %d\n",Dir,Sound,(int)Loop);
}

void TEventMP3::copyTo(TEventMP3 *dist){
    TEvent::copyTo(dist);
    dist->setSound(Dir,Sound,Loop);
}

